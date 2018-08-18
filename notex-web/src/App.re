open Belt;

[@bs.module] external styles : Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name) |. Option.getExn;

open Data;

type state = {
  notebooks: option(list(notebook)),
  selectedNotebook: option(selectedNotebook),
  selectedNote: option(selectedNote),
};

type action =
  | LoadInitialState
  | LoadNotebooks(list(notebook))
  | LoadNotebook(notebook)
  | LoadNote(note)
  | SelectNotebook(selectedNotebook)
  | SelectNote(selectedNote)
  | UpdateNoteText(note, content, string);

let mapDbNotebook = ((notebook: Db.notebook, noteCount)) => {
  id: notebook.id,
  name: notebook.name,
  createdAt: notebook.createdAt,
  noteCount,
};

let mapDbNote = (dbNote: Db.note) : note => {
  id: dbNote.id,
  title: dbNote.title,
  tags: dbNote.tags,
  createdAt: dbNote.createdAt,
  updatedAt: dbNote.updatedAt,
};

let mapDbContentBlock = (dbContentBlock: Db.contentBlock) : contentBlock => {
  id: dbContentBlock.id,
  content:
    switch (dbContentBlock.content) {
    | Db.TextContent(text) => TextContent(text)
    | Db.CodeContent(code, language) => CodeContent(code, language)
    },
};

module MainUI = {
  let renderNotebooks =
      (
        notebooks: list(notebook),
        selectedNotebook: option(selectedNotebook),
        send,
      ) => {
    let listItems =
      notebooks
      |. List.map(notebook =>
           (
             {
               id: notebook.id |> string_of_int,
               title: notebook.name,
               count: Some(notebook.noteCount),
               model: notebook,
             }:
               ListView.listItem(notebook)
           )
         );

    <ListView
      items=listItems
      selectedId=(
        selectedNotebook
        |. Option.map(selected => selected.notebook.id |. string_of_int)
      )
      onItemSelected=(item => send(LoadNotebook(item.model)))
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
        selected.notes
        |. List.map(note =>
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
        (ReasonReact.string(item.model.title))
        <br />
        <small>
          (ReasonReact.string(item.model.updatedAt |> formatDate))
        </small>
      </p>;

    <ListView
      items=listItems
      selectedId=(
        editingNote
        |. Option.map(selectedNote => selectedNote.note.id |. string_of_int)
      )
      onItemSelected=(item => send(LoadNote(item.model)))
      renderItemContent=renderNoteListItemContent
    />;
  };

  let component = ReasonReact.statelessComponent("MainUI");
  let make = (~notebooks, ~selectedNotebook, ~editingNote, ~send, _children) => {
    ...component,
    render: _self =>
      <main className=(style("main"))>
        <div className=(style("columns"))>
          (renderNotebooks(notebooks, selectedNotebook, send))
          (renderNotes(selectedNotebook, editingNote, send))
          <NoteEditor
            note=editingNote
            onChange=((_, _, _) => Js.log("TODO"))
          />
        </div>
      </main>,
  };
};

let reducer = (action: action, state: state) =>
  switch (action) {
  | LoadInitialState =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getNotebooks()
          |. Future.get(notebooks => {
               let notebooks = notebooks |. List.map(mapDbNotebook);
               self.send(LoadNotebooks(notebooks));
             })
      ),
    )

  | LoadNotebooks(notebooks) =>
    let newState = {...state, notebooks: Some(notebooks)};

    ReasonReact.UpdateWithSideEffects(
      newState,
      (
        self => {
          let firstNotebook = notebooks |. List.head;

          switch (firstNotebook) {
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
          |. Future.get(notes => {
               let notes = notes |. List.map(mapDbNote);

               self.send(SelectNotebook({notebook, notes}));

               switch (notes |. List.head) {
               | None => ()
               | Some(note) => self.send(LoadNote(note))
               };
             })
      ),
    )
  | LoadNote(note) =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getContentBlocks(note.id)
          |. Future.get(dbContentBlocks => {
               let contentBlocks =
                 List.map(dbContentBlocks, mapDbContentBlock);

               {note, content: contentBlocks} |. SelectNote |. self.send;
             })
      ),
    )

  | SelectNotebook(selectedNotebook) =>
    {...state, selectedNotebook: Some(selectedNotebook)}
    |. ReasonReact.Update

  | SelectNote(selectedNote) =>
    {...state, selectedNote: Some(selectedNote)} |. ReasonReact.Update

  | UpdateNoteText(_note, _content, _text) => ReasonReact.NoUpdate
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
    Db.subscribe(() => self.send(LoadInitialState));

    self.send(LoadInitialState);
  },
  render: self =>
    switch (self.state.notebooks) {
    | None => <div> (ReasonReact.string("Loading...")) </div>
    | Some(_notebooks) =>
      <MainUI
        notebooks=(self.state.notebooks |. Option.getExn)
        selectedNotebook=self.state.selectedNotebook
        editingNote=self.state.selectedNote
        send=self.send
      />
    },
};
