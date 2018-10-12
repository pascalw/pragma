open Belt;

[@bs.module] external styles: Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

open Data;

type noteCount = int;
type notebookWithCount = (notebook, noteCount);

type state = {
  notebooks: option(list(notebookWithCount)),
  selectedNotebook: option(string),
  notebookIdEditingTitle: option(string),
  notes: option(list(note)),
  selectedNote: option(string),
  contentBlocks: option(list(contentBlock)),
};

type action =
  | Load(state)
  | SelectNote(string)
  | LoadNote(string, list(contentBlock))
  | CreateNotebook
  | DeleteNotebook(notebook)
  | CreateNote
  | DeleteNote(note)
  | SelectNotebook(string)
  | UpdateNotebook(notebook)
  | LoadNotebook(
      string,
      list(note),
      option(string),
      option(list(contentBlock)),
    )
  | UpdateNoteText(contentBlock, string)
  | UpdateNoteTitle(note, string)
  | EditNotebookTitle(notebook);

let sortDesc = (notes: list(note)) =>
  List.sort(notes, (a, b) =>
    Utils.compareDates(a.updatedAt, b.updatedAt) * (-1)
  );

let getNotes = notebookId =>
  Db.getNotes(notebookId)
  ->Future.map(notes => {
      let selectedNoteId = List.head(notes)->Option.map(note => note.id);
      (notes, selectedNoteId);
    })
  ->Future.flatMap(((notes, selectedNoteId)) =>
      switch (selectedNoteId) {
      | None => Future.value((notes, selectedNoteId, None))
      | Some(noteId) =>
        Db.getContentBlocks(noteId)
        ->Future.map(contentBlocks =>
            (notes, selectedNoteId, Some(contentBlocks))
          )
      }
    );

module MainUI = {
  let renderNotebooks =
      (
        notebooks: list(notebookWithCount),
        selectedNotebook: option(string),
        notebookIdEditingTitle: option(string),
        send,
      ) => {
    let listFooter = <> <AddButton onClick={_ => send(CreateNotebook)} /> </>;
    let listItems =
      List.map(notebooks, ((notebook, noteCount)) =>
        (
          {
            id: notebook.id,
            title: notebook.title,
            count: Some(noteCount),
            model: notebook,
          }:
            ListView.listItem(notebook)
        )
      );

    let renderNotebookListItemContent =
        (send, item: ListView.listItem(notebook)) =>
      if (Some(item.model.id) == notebookIdEditingTitle) {
        <p>
          <NotebookTitleEditor
            value={item.model.title}
            onComplete={
              title => {
                let updatedNotebook = {...item.model, title};
                send(UpdateNotebook(updatedNotebook));
              }
            }
          />
        </p>;
      } else {
        ListView.defaultRenderItemContent(item);
      };

    <ListView
      items=listItems
      selectedId=selectedNotebook
      onItemSelected={item => send(SelectNotebook(item.model.id))}
      onItemDoubleClick={item => send(EditNotebookTitle(item.model))}
      onItemLongpress={
        item =>
          if (WindowRe.confirm(
                "Are you sure you want to delete this notebook?",
                Webapi.Dom.window,
              )) {
            send(DeleteNotebook(item.model));
          }
      }
      renderItemContent={renderNotebookListItemContent(send)}
      renderFooter={() => listFooter}
    />;
  };

  let renderNotes =
      (
        selectedNotebook: option(string),
        notes: option(list(note)),
        selectedNote: option(string),
        send,
      ) => {
    let renderFooter =
      switch (selectedNotebook) {
      | None => (() => ReasonReact.null)
      | Some(_) => (
          () => <> <AddButton onClick={_ => send(CreateNote)} /> </>
        )
      };

    let listItems =
      switch (notes) {
      | None => []
      | Some(notes) =>
        notes
        ->sortDesc
        ->List.map(note =>
            (
              {id: note.id, title: note.title, count: None, model: note}:
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
      selectedId=selectedNote
      onItemSelected={item => send(SelectNote(item.model.id))}
      onItemLongpress={
        item =>
          if (WindowRe.confirm(
                "Are you sure you want to delete this note?",
                Webapi.Dom.window,
              )) {
            send(DeleteNote(item.model));
          }
      }
      renderItemContent=renderNoteListItemContent
      renderFooter
    />;
  };

  let onChangeNoteTextDebounced = send => {
    let timerId: ref(option(Js.Global.timeoutId)) = ref(None);

    (contentBlock, value) => {
      switch (timerId^) {
      | Some(timerId) => Js.Global.clearTimeout(timerId)
      | _ => ()
      };

      timerId :=
        Some(
          Js.Global.setTimeout(
            () => send(UpdateNoteText(contentBlock, value)),
            1000,
          ),
        );
    };
  };

  let onChange = (send, onChangeNoteText, change) =>
    switch (change) {
    | NoteEditor.Text(contentBlock, value) =>
      onChangeNoteText(contentBlock, value)
    | NoteEditor.Title(note, title) => send(UpdateNoteTitle(note, title))
    };

  let component = ReasonReact.statelessComponent("MainUI");
  let make =
      (
        ~notebooks,
        ~selectedNotebook,
        ~notebookIdEditingTitle,
        ~notes,
        ~selectedNote,
        ~contentBlocks,
        ~editingNote,
        ~send,
        _children,
      ) => {
    ...component,
    render: _self =>
      <main className={style("main")}>
        <div className={style("columns")}>
          {
            renderNotebooks(
              notebooks,
              selectedNotebook,
              notebookIdEditingTitle,
              send,
            )
          }
          {renderNotes(selectedNotebook, notes, selectedNote, send)}
          {
            switch (editingNote) {
            | None => <NoNoteSelected />
            | Some(_) =>
              <NoteEditor
                note=editingNote
                contentBlocks
                onChange={onChange(send, onChangeNoteTextDebounced(send))}
              />
            }
          }
        </div>
      </main>,
  };
};

let reducer = (action: action, state: state) =>
  switch (action) {
  | Load(state) => ReasonReact.Update(state)
  | SelectNotebook(notebookId) =>
    ReasonReact.SideEffects(
      (
        self =>
          getNotes(notebookId)
          ->Future.get(((notes, selectedNoteId, contentBlocks)) =>
              self.send(
                LoadNotebook(
                  notebookId,
                  notes,
                  selectedNoteId,
                  contentBlocks,
                ),
              )
            )
      ),
    )

  | LoadNotebook(notebookId, notes, selectedNoteId, contentBlocks) =>
    {
      ...state,
      selectedNotebook: Some(notebookId),
      notes: Some(notes),
      selectedNote: selectedNoteId,
      contentBlocks,
    }
    ->ReasonReact.Update

  | SelectNote(noteId) =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.getContentBlocks(noteId)
          ->Future.get(contentBlocks =>
              self.send(LoadNote(noteId, contentBlocks))
            )
      ),
    )

  | LoadNote(noteId, contentBlocks) =>
    {
      ...state,
      selectedNote: Some(noteId),
      contentBlocks: Some(contentBlocks),
    }
    ->ReasonReact.Update
  | CreateNote =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.withNotification(() =>
            Db.createNote(self.state.selectedNotebook |> Option.getExn)
          )
          ->Future.map(((note, _contentBlock)) =>
              self.send(SelectNote(note.id))
            )
          ->ignore
      ),
    )
  | DeleteNote(note) =>
    let updatedNotes =
      Belt.List.keep(state.notes |> Option.getExn, existingNote =>
        existingNote.id != note.id
      );

    let newState = {...state, notes: Some(updatedNotes)};
    ReasonReact.UpdateWithSideEffects(
      newState,
      (
        _self =>
          Db.withNotification(() => Db.deleteNote(note.id, ()) |> ignore)
      ),
    );
  | CreateNotebook =>
    ReasonReact.SideEffects(
      (
        self =>
          Db.withNotification(() => Db.createNotebook())
          ->Future.map(notebook => {
              self.send(SelectNotebook(notebook.id));
              self.send(EditNotebookTitle(notebook));
            })
          ->ignore
      ),
    )
  | DeleteNotebook(notebook) =>
    let updatedNotebooks =
      Belt.List.keep(
        state.notebooks |> Option.getExn, ((existingNotebook, _)) =>
        existingNotebook.id != notebook.id
      );

    let newState = {...state, notebooks: Some(updatedNotebooks)};
    ReasonReact.UpdateWithSideEffects(
      newState,
      (
        _self =>
          Db.withNotification(() => Db.deleteNotebook(notebook.id, ()))
          |> ignore
      ),
    );

  | UpdateNotebook(notebook) =>
    let updatedNotebooks =
      Belt.List.map(
        state.notebooks |> Option.getExn, ((existingNotebook, noteCount)) =>
        if (existingNotebook.id == notebook.id) {
          (notebook, noteCount);
        } else {
          (existingNotebook, noteCount);
        }
      );

    let newState = {
      ...state,
      notebookIdEditingTitle: None,
      notebooks: Some(updatedNotebooks),
    };

    ReasonReact.UpdateWithSideEffects(
      newState,
      (_self => Db.updateNotebook(notebook, ()) |> ignore),
    );

  | UpdateNoteText(contentBlock, text) =>
    let updatedContentBlock =
      switch (contentBlock.content) {
      | TextContent(_) => {...contentBlock, content: TextContent(text)}
      | CodeContent(_code, language) => {
          ...contentBlock,
          content: CodeContent(text, language),
        }
      };

    ReasonReact.SideEffects(
      (_self => Db.updateContentBlock(updatedContentBlock, ()) |> ignore),
    );
  | UpdateNoteTitle(note, title) =>
    let updatedNote = {...note, title};

    let updatedNotes =
      Belt.List.map(state.notes |> Option.getExn, existingNote =>
        if (existingNote.id == note.id) {
          updatedNote;
        } else {
          existingNote;
        }
      );

    let newState = {...state, notes: Some(updatedNotes)};
    ReasonReact.UpdateWithSideEffects(
      newState,
      (_self => Db.updateNote(updatedNote, ()) |> ignore),
    );
  | EditNotebookTitle(notebook) =>
    ReasonReact.Update({...state, notebookIdEditingTitle: Some(notebook.id)})
  };

let component = ReasonReact.reducerComponent("App");
let make = _children => {
  ...component,
  initialState: () => {
    notebooks: None,
    selectedNotebook: None,
    notebookIdEditingTitle: None,
    notes: None,
    selectedNote: None,
    contentBlocks: None,
  },
  reducer,
  didMount: self => {
    let fetchData = () => {
      let appState = AppState.get();

      Db.getNotebooks()
      ->Future.flatMap(notebooks => {
          let selectedNotebookId =
            switch (appState.selectedNotebookId) {
            | Some(id) =>
              notebooks
              ->List.map(((notebook, _count)) => notebook)
              ->Utils.find(notebook => notebook.id == id)
              ->Option.map(notebook => notebook.id)
            | None =>
              List.head(notebooks)
              ->Option.map(((notebook, _count)) => notebook.id)
            };

          switch (selectedNotebookId) {
          | None => Future.value((notebooks, None, None))
          | Some(notebookId) =>
            Db.getNotes(notebookId)
            ->Future.map(notes =>
                (notebooks, Some(notebookId), Some(notes))
              )
          };
        })
      ->Future.get(((notebooks, selectedNotebook, notes)) => {
          let selectedNoteId =
            switch (appState.selectedNoteId) {
            | Some(id) => Some(id)
            | None =>
              Option.flatMap(notes, notes =>
                List.head(notes)->Option.map(note => note.id)
              )
            };

          let contentBlocksFuture =
            switch (selectedNoteId) {
            | None => Future.value([])
            | Some(noteId) => Db.getContentBlocks(noteId)
            };

          contentBlocksFuture
          ->Future.get(contentBlocks =>
              self.send(
                Load({
                  notebooks: Some(notebooks),
                  notes,
                  selectedNotebook,
                  notebookIdEditingTitle: None,
                  selectedNote: selectedNoteId,
                  contentBlocks: Some(contentBlocks),
                }),
              )
            );
        });
    };

    fetchData();

    Db.subscribe(fetchData);
    self.onUnmount(() => Db.unsubscribe(fetchData));
  },
  didUpdate: ({oldSelf: _oldSelf, newSelf}) =>
    AppState.setSelected(
      newSelf.state.selectedNotebook,
      newSelf.state.selectedNote,
    ),
  render: self => {
    let editingNote =
      self.state.selectedNote
      ->Option.flatMap(selectedNote =>
          switch (self.state.notes) {
          | None => None
          | Some(notes) => Utils.find(notes, n => n.id == selectedNote)
          }
        );

    switch (self.state.notebooks) {
    | None => <div> {ReasonReact.string("Loading...")} </div>
    | Some(_notebooks) =>
      <MainUI
        notebooks=self.state.notebooks->Option.getExn
        selectedNotebook={self.state.selectedNotebook}
        notebookIdEditingTitle={self.state.notebookIdEditingTitle}
        notes={self.state.notes}
        selectedNote={self.state.selectedNote}
        contentBlocks={self.state.contentBlocks}
        editingNote
        send={self.send}
      />
    };
  },
};
