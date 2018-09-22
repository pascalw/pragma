open Belt;

[@bs.module] external styles: Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

open Data;

type noteCount = int;
type notebookWithCount = (notebook, noteCount);

type state = {
  notebooks: option(list(notebookWithCount)),
  selectedNotebook: option(selectedNotebook),
  selectedNote: option(selectedNote),
};

type action =
  | ReloadDbState
  | LoadNotebooks(list(notebookWithCount))
  | LoadNotebook(notebook)
  | LoadNote(note)
  | SelectNotebook(selectedNotebook)
  | SelectNote(selectedNote)
  | UpdateNoteText(contentBlock, string);

let sortDesc = (notes: list(note)) =>
  Belt.List.sort(notes, (a, b) =>
    Utils.compareDates(a.updatedAt, b.updatedAt) * (-1)
  );

module MainUI = {
  let renderNotebooks =
      (
        notebooks: list(notebookWithCount),
        selectedNotebook: option(selectedNotebook),
        send,
      ) => {
    let listItems =
      List.map(notebooks, ((notebook, noteCount)) =>
        (
          {
            id: notebook.id |> string_of_int,
            title: notebook.name,
            count: Some(noteCount),
            model: notebook,
          }:
            ListView.listItem(notebook)
        )
      );

    <ListView
      items=listItems
      selectedId={
        Option.map(selectedNotebook, selected =>
          selected.notebook.id->string_of_int
        )
      }
      onItemSelected={item => send(LoadNotebook(item.model))}
    />;
  };

  let renderNotes =
      (
        selectedNotebook: option(selectedNotebook),
        editingNote: option(selectedNote),
        send,
      ) => {
    let listItems =
      switch (selectedNotebook) {
      | None => []
      | Some(selected) =>
        List.map(selected.notes, note =>
          (
            {
              id: note.id |> string_of_int,
              title: note.title,
              count: None,
              model: note,
            }:
              ListView.listItem(note)
          )
        )
      };

    let formatDate = date => DateFns.format("D MMMM YYYY", date);
    let renderNoteListItemContent = (item: ListView.listItem(note)) =>
      <p>
        {ReasonReact.string(item.model.title)}
        <br />
        <small>
          {ReasonReact.string(item.model.updatedAt |> formatDate)}
        </small>
      </p>;

    <ListView
      minWidth="250px"
      items=listItems
      selectedId={
        Option.map(editingNote, selectedNote =>
          selectedNote.note.id->string_of_int
        )
      }
      onItemSelected={item => send(LoadNote(item.model))}
      renderItemContent=renderNoteListItemContent
    />;
  };

  let component = ReasonReact.statelessComponent("MainUI");
  let make = (~notebooks, ~selectedNotebook, ~editingNote, ~send, _children) => {
    ...component,
    render: _self =>
      <main className={style("main")}>
        <div className={style("columns")}>
          {renderNotebooks(notebooks, selectedNotebook, send)}
          {renderNotes(selectedNotebook, editingNote, send)}
          <NoteEditor
            note=editingNote
            onChange={
              (contentBlock, value) =>
                send(UpdateNoteText(contentBlock, value))
            }
          />
        </div>
      </main>,
  };
};

let reducer = (action: action, state: state) =>
  switch (action) {
  | ReloadDbState =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getNotebooks()
          ->(Future.get(notebooks => self.send(LoadNotebooks(notebooks))))
      ),
    )
  | LoadNotebooks(notebooks) =>
    let newState = {...state, notebooks: Some(notebooks)};

    ReasonReact.UpdateWithSideEffects(
      newState,
      (
        self => {
          let selectedNotebook =
            if (Option.isSome(state.selectedNotebook)) {
              Option.map(state.selectedNotebook, selectedNotebook =>
                selectedNotebook.notebook
              );
            } else {
              notebooks
              ->List.head
              ->Belt.Option.map(((notebook, _)) => notebook);
            };

          switch (selectedNotebook) {
          | None => ()
          | Some(notebook) => self.send(LoadNotebook(notebook))
          };
        }
      ),
    );
  | LoadNotebook(notebook) =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getNotes(notebook.id)
          ->Future.map(sortDesc)
          ->(
              Future.get(notes => {
                self.send(SelectNotebook({notebook, notes}));

                let isDifferentNotebook =
                  switch (state.selectedNotebook) {
                  | Some(selectedNotebook) =>
                    selectedNotebook.notebook.id != notebook.id
                  | None => true
                  };

                let selectedNote =
                  if (isDifferentNotebook || Option.isNone(state.selectedNote)) {
                    List.head(notes);
                  } else {
                    Option.map(state.selectedNote, selectedNote =>
                      selectedNote.note
                    );
                  };

                switch (selectedNote) {
                | None => ()
                | Some(note) => self.send(LoadNote(note))
                };
              })
            )
      ),
    )
  | LoadNote(note) =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getContentBlocks(note.id)
          ->(
              Future.get(contentBlocks =>
                {note, content: contentBlocks}->SelectNote->(self.send)
              )
            )
      ),
    )

  | SelectNotebook(selectedNotebook) =>
    {...state, selectedNotebook: Some(selectedNotebook)}->ReasonReact.Update

  | SelectNote(selectedNote) =>
    {...state, selectedNote: Some(selectedNote)}->ReasonReact.Update

  | UpdateNoteText(contentBlock, text) =>
    let updatedContentBlock =
      switch (contentBlock.content) {
      | TextContent(_) => {...contentBlock, content: TextContent(text)}
      | _ => Js.Exn.raiseError("TODO")
      };

    ReasonReact.SideEffects(
      (_self => Db.updateContentBlock(updatedContentBlock, ()) |> ignore),
    );
  };

let component = ReasonReact.reducerComponent("App");
let make = _children => {
  ...component,
  initialState: () => {
    notebooks: None,
    selectedNotebook: None,
    selectedNote: None,
  },
  reducer,
  didMount: self => {
    Db.subscribe(() => self.send(ReloadDbState));

    self.send(ReloadDbState);
  },
  render: self =>
    switch (self.state.notebooks) {
    | None => <div> {ReasonReact.string("Loading...")} </div>
    | Some(_notebooks) =>
      <MainUI
        notebooks=self.state.notebooks->Option.getExn
        selectedNotebook={self.state.selectedNotebook}
        editingNote={self.state.selectedNote}
        send={self.send}
      />
    },
};
