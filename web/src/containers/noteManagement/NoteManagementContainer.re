type state = {
  initialStateLoaded: bool,
  notebooks: list((Data.notebook, int)),
  notes: list(Data.note),
  contentBlocks: list(Data.contentBlock),
  selectedNotebook: option(string),
  selectedNote: option(string),
};

type action =
  | LoadInitialState(state)
  | CreateNotebook(Data.notebook)
  | UpdateNotebook(Data.notebook)
  | SelectNotebook(Data.notebook)
  | NotebookSelected(
      list(Data.note),
      option(string),
      list(Data.contentBlock),
    )
  | DeleteNotebook(Data.notebook)
  | CreateNote
  | SelectNote(string)
  | NoteSelected(string, list(Data.contentBlock))
  | DeleteNote(Data.note)
  | UpdateNoteText(Data.contentBlock, Data.content)
  | UpdateContentBlock(Data.contentBlock)
  | UpdateNoteTitle(Data.note, string);

let debouncedUpdateContentBlockInDb =
  (
    () => {
      let timerId: ref(option(Js.Global.timeoutId)) = ref(None);

      contentBlock => {
        switch (timerId^) {
        | Some(timerId) => Js.Global.clearTimeout(timerId)
        | _ => ()
        };

        timerId :=
          Some(
            Js.Global.setTimeout(
              () => ContentBlocks.update(contentBlock, ()) |> ignore,
              500,
            ),
          );
      };
    }
  )();

let sortDesc = (input: list('a), mapper: 'a => Js.Date.t) =>
  Belt.List.sort(input, (a, b) =>
    Utils.compareDates(mapper(a), mapper(b)) * (-1)
  );

let sortNotesDesc = (notes: list(Data.note)) =>
  sortDesc(notes, note => note.updatedAt);

let sortNotebooksDesc = (notebooks: list((Data.notebook, int))) =>
  sortDesc(notebooks, ((notebook, _)) => notebook.updatedAt);

let getSortedNotes = notebookId =>
  Notes.fromNotebook(notebookId)->Future.map(sortNotesDesc);

let fetchInitialState = () => {
  open Belt;
  let appState = AppState.get();

  Notebooks.all()
  ->Future.map(sortNotebooksDesc)
  ->Future.flatMap(notebooks => {
      let selectedNotebookId =
        switch (appState.selectedNotebookId) {
        | Some(id) =>
          notebooks
          ->List.map(((notebook, _count)) => notebook)
          ->Utils.find(notebook => notebook.id == id)
          ->Belt.Option.map(notebook => notebook.id)
        | None =>
          List.head(notebooks)
          ->Option.map(((notebook, _count)) => notebook.id)
        };

      switch (selectedNotebookId) {
      | None => Future.value((notebooks, None, []))
      | Some(notebookId) =>
        getSortedNotes(notebookId)
        ->Future.map(notes => (notebooks, Some(notebookId), notes))
      };
    })
  ->Future.flatMap(((notebooksWithCounts, selectedNotebook, notes)) => {
      let selectedNoteId =
        switch (appState.selectedNoteId) {
        | Some(id) => Some(id)
        | None => List.head(notes)->Option.map(note => note.id)
        };

      let contentBlocksFuture =
        switch (selectedNoteId) {
        | None => Future.value([])
        | Some(noteId) => ContentBlocks.fromNote(noteId)
        };

      contentBlocksFuture->Future.map(contentBlocks =>
        {
          initialStateLoaded: true,
          notebooks: notebooksWithCounts,
          notes,
          contentBlocks,
          selectedNotebook,
          selectedNote: selectedNoteId,
        }
      );
    });
};

let getNotes = notebookId =>
  getSortedNotes(notebookId)
  ->Future.map(notes => {
      let selectedNoteId =
        Belt.List.head(notes)->Belt.Option.map(note => note.id);
      (notes, selectedNoteId);
    })
  ->Future.flatMap(((notes, selectedNoteId)) =>
      switch (selectedNoteId) {
      | None => Future.value((notes, selectedNoteId, None))
      | Some(noteId) =>
        ContentBlocks.fromNote(noteId)
        ->Future.map(contentBlocks =>
            (notes, selectedNoteId, Some(contentBlocks))
          )
      }
    );

let component = ReasonReact.reducerComponent("NoteManagementContainer");
let make = (children: (state, action => unit) => ReasonReact.reactElement) => {
  ...component,
  initialState: () => {
    initialStateLoaded: false,
    notebooks: [],
    notes: [],
    contentBlocks: [],
    selectedNotebook: None,
    selectedNote: None,
  },
  reducer: (action: action, state: state) =>
    switch (action) {
    | LoadInitialState(state) => ReasonReact.Update(state)
    | SelectNotebook(notebook) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, selectedNotebook: Some(notebook.id)},
        (
          self =>
            getNotes(notebook.id)
            ->Future.get(((notes, selectedNoteId, contentBlocks)) =>
                self.send(
                  NotebookSelected(
                    notes,
                    selectedNoteId,
                    Belt.Option.getWithDefault(contentBlocks, []),
                  ),
                )
              )
        ),
      )
    | NotebookSelected(notes, selectedNoteId, contentBlocks) =>
      ReasonReact.Update({
        ...state,
        notes,
        contentBlocks,
        selectedNote: selectedNoteId,
      })
    | CreateNotebook(notebook) =>
      let updatedNotebooks = Belt.List.add(state.notebooks, (notebook, 0));
      let newState = {...state, notebooks: updatedNotebooks};

      ReasonReact.UpdateWithSideEffects(
        newState,
        (
          self =>
            Db.withNotification(() => Notebooks.create(notebook))
            ->Future.map(notebook => self.send(SelectNotebook(notebook)))
            ->ignore
        ),
      );
    | DeleteNotebook(notebook) =>
      let updatedNotebooks =
        Belt.List.keep(state.notebooks, ((existingNotebook, _noteCount)) =>
          existingNotebook.id != notebook.id
        );

      let newState = {...state, notebooks: updatedNotebooks};
      ReasonReact.UpdateWithSideEffects(
        newState,
        (
          _self =>
            Db.withNotification(() => Notebooks.delete(notebook.id, ()))
            |> ignore
        ),
      );
    | UpdateNotebook(notebook) =>
      let updatedNotebooks =
        Belt.List.map(state.notebooks, ((existingNotebook, noteCount)) =>
          if (existingNotebook.id == notebook.id) {
            (notebook, noteCount);
          } else {
            (existingNotebook, noteCount);
          }
        );

      let newState = {...state, notebooks: updatedNotebooks};

      ReasonReact.UpdateWithSideEffects(
        newState,
        (_self => Notebooks.update(notebook, ()) |> ignore),
      );
    | SelectNote(noteId) =>
      ReasonReact.SideEffects(
        (
          self =>
            ContentBlocks.fromNote(noteId)
            ->Future.get(contentBlocks =>
                self.send(NoteSelected(noteId, contentBlocks))
              )
        ),
      )
    | NoteSelected(noteId, contentBlocks) =>
      ReasonReact.Update({
        ...state,
        selectedNote: Some(noteId),
        contentBlocks,
      })
    | CreateNote =>
      ReasonReact.SideEffects(
        (
          self =>
            Db.withNotification(() =>
              Notes.create(self.state.selectedNotebook |> Belt.Option.getExn)
            )
            ->Future.map(((note, _contentBlock)) =>
                self.send(SelectNote(note.id))
              )
            ->ignore
        ),
      )
    | DeleteNote(note) =>
      let updatedNotes =
        Belt.List.keep(state.notes, existingNote =>
          existingNote.id != note.id
        );

      let newState = {...state, notes: updatedNotes};
      ReasonReact.UpdateWithSideEffects(
        newState,
        (
          _self =>
            Db.withNotification(() => Notes.delete(note.id, ()) |> ignore)
        ),
      );
    | UpdateNoteText(contentBlock, content) =>
      let updatedContentBlock = {...contentBlock, content};
      let updatedBlocks =
        Belt.List.map(state.contentBlocks, existingBlock =>
          if (existingBlock.id == updatedContentBlock.id) {
            updatedContentBlock;
          } else {
            existingBlock;
          }
        );
      let newState = {...state, contentBlocks: updatedBlocks};

      ReasonReact.UpdateWithSideEffects(
        newState,
        (_self => debouncedUpdateContentBlockInDb(updatedContentBlock)),
      );
    | UpdateContentBlock(updatedBlock) =>
      let updatedBlocks =
        Belt.List.map(state.contentBlocks, existingBlock =>
          if (existingBlock.id == updatedBlock.id) {
            updatedBlock;
          } else {
            existingBlock;
          }
        );

      let newState = {...state, contentBlocks: updatedBlocks};
      ReasonReact.UpdateWithSideEffects(
        newState,
        (_self => ContentBlocks.update(updatedBlock, ()) |> ignore),
      );
    | UpdateNoteTitle(note, title) =>
      let updatedNote = {...note, title};

      let updatedNotes =
        Belt.List.map(state.notes, existingNote =>
          if (existingNote.id == note.id) {
            updatedNote;
          } else {
            existingNote;
          }
        );

      let newState = {...state, notes: updatedNotes};
      ReasonReact.UpdateWithSideEffects(
        newState,
        (_self => Notes.update(updatedNote, ()) |> ignore),
      );
    },
  didMount: self => {
    DbSync.run();
    DataSyncRetry.getPendingChanges()->Future.get(DataSync.start);

    let loadStateFromDb = () =>
      fetchInitialState()
      ->Future.get(state => self.send(LoadInitialState(state)));

    loadStateFromDb();
    Db.subscribe(loadStateFromDb);
    self.onUnmount(() => Db.unsubscribe(loadStateFromDb));
  },
  didUpdate: ({oldSelf: _oldSelf, newSelf}) =>
    AppState.setSelected(
      newSelf.state.selectedNotebook,
      newSelf.state.selectedNote,
    ),
  render: self =>
    <>
      {
        switch (self.state.initialStateLoaded) {
        | false => ReasonReact.null
        | true => children(self.state, self.send)
        }
      }
    </>,
};
