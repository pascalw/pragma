open Common;
open UiTypes;

let maxRecentNotes = 10;

let refreshDataMinElapsed = 10.0 *. 1000.0;
let lastDataRefresh: ref(option(Js.Date.t)) = ref(None);

type state = {
  initialStateLoaded: bool,
  notebooks: list((Data.notebook, int)),
  noteCollections: list(NoteCollection.Collection.t),
  notes: list(Data.note),
  contentBlocks: list(Data.contentBlock),
  selectedCollection: option(string),
  selectedNote: option(string),
};

type action =
  | ReloadState
  | LoadState(state)
  | CreateNotebook(Data.notebook)
  | UpdateNotebook(Data.notebook)
  | SelectNotebook(NoteCollection.t)
  | NotebookSelected(list(Data.note), option(string))
  | DeleteNotebook(Data.notebook)
  | CreateNote
  | SelectNote(string)
  | NoteSelected(string, list(Data.contentBlock))
  | DeleteNote(Data.note)
  | UpdateNoteText(Data.contentBlock, Data.content)
  | UpdateContentBlock(Data.contentBlock)
  | UpdateNoteTitle(Data.note, string);

let selectedCollection =
    (noteCollections, notebooks: list((Data.notebook, int)), selectedCollectionId)
    : option(NoteCollection.t) => {
  let collection =
    Utils.find(noteCollections, collection =>
      Some(NoteCollection.Collection.id(collection)) == selectedCollectionId
    );

  switch (collection) {
  | Some(_) => collection >>= NoteCollection.fromCollection
  | None =>
    Utils.find(notebooks, ((notebook, _)) => Some(notebook.id) == selectedCollectionId)
    >>= (((notebook, _)) => NoteCollection.fromNotebook(notebook))
  };
};

let selectedCollectionFromState = (state: state): option(NoteCollection.t) => {
  selectedCollection(state.noteCollections, state.notebooks, state.selectedCollection);
};

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
            Js.Global.setTimeout(() => ContentBlocks.update(contentBlock, ()) |> ignore, 500),
          );
      };
    }
  )();

let sortDesc = (input: list('a), mapper: 'a => Js.Date.t) =>
  Belt.List.sort(input, (a, b) => Utils.compareDates(mapper(a), mapper(b)) * (-1));

let sortNotesDesc = (notes: list(Data.note)) => sortDesc(notes, note => note.updatedAt);

let sortNotebooksDesc = (notebooks: list((Data.notebook, int))) =>
  sortDesc(notebooks, ((notebook, _)) => notebook.updatedAt);

let getSortedNotes = notebookId =>
  Notes.fromNotebook(notebookId) |> Repromise.map(sortNotesDesc);

let fetchAllData = () => {
  open Belt;
  let appState = AppState.get();

  Notebooks.all()
  |> Repromise.map(sortNotebooksDesc)
  |> Repromise.andThen((notebooks: list((Data.notebook, int))) =>
       Db.getRecentNotesCount()
       |> Repromise.map(count => {
            let count = count > maxRecentNotes ? maxRecentNotes : count;
            let collection =
              NoteCollection.makeCollection(NoteCollection.CollectionKind.Recents, count);

            (notebooks, [collection]);
          })
     )
  |> Repromise.andThen(((notebooks: list((Data.notebook, int)), noteCollections)) => {
       let selectedCollection =
         selectedCollection(noteCollections, notebooks, appState.selectedNotebookId);
       let selectedCollectionId = selectedCollection >>= NoteCollection.id;

       switch (selectedCollection) {
       | Some(NoteCollection.Notebook(notebook)) =>
         getSortedNotes(notebook.id)
         |> Repromise.map(notes => (notebooks, selectedCollectionId, notes, noteCollections))
       | Some(NoteCollection.Collection({kind: NoteCollection.CollectionKind.Recents})) =>
         Db.getRecentNotes(maxRecentNotes)
         |> Repromise.map(notes => (notebooks, selectedCollectionId, notes, noteCollections))
       | None => Repromise.resolved((notebooks, None, [], noteCollections))
       };
     })
  |> Repromise.andThen(
       ((notebooksWithCounts, selectedCollection, notes: list(Data.note), noteCollections)) => {
       let selectedNoteId =
         switch (appState.selectedNoteId) {
         | Some(id) => Some(id)
         | None => List.head(notes)->Option.map(note => note.id)
         };

       let contentBlocksPromise =
         switch (selectedNoteId) {
         | None => Repromise.resolved([])
         | Some(noteId) => ContentBlocks.fromNote(noteId)
         };

       contentBlocksPromise
       |> Repromise.map(contentBlocks =>
            {
              initialStateLoaded: true,
              notebooks: notebooksWithCounts,
              noteCollections,
              notes,
              contentBlocks,
              selectedCollection,
              selectedNote: selectedNoteId,
            }
          );
     });
};

let selectFirstNote = notes =>
  notes
  |> Repromise.map((notes: list(Data.note)) => {
       let selectedNoteId = Belt.List.head(notes)->Belt.Option.map(note => note.id);
       (notes, selectedNoteId);
     });

let getNotes = notebookId => getSortedNotes(notebookId) |> selectFirstNote;

let component = ReasonReact.reducerComponent("NoteManagementContainer");
let make = (children: (state, action => unit) => ReasonReact.reactElement) => {
  ...component,
  initialState: () => {
    initialStateLoaded: false,
    notebooks: [],
    noteCollections: [],
    notes: [],
    contentBlocks: [],
    selectedCollection: None,
    selectedNote: None,
  },
  reducer: (action: action, state: state) =>
    switch (action) {
    | ReloadState =>
      ReasonReact.SideEffects(
        self => fetchAllData() |> Repromise.wait(state => self.send(LoadState(state))),
      )
    | LoadState(state) => ReasonReact.Update(state)
    | SelectNotebook(
        NoteCollection.Collection({kind: NoteCollection.CollectionKind.Recents} as collection),
      ) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, selectedCollection: Some(NoteCollection.Collection.id(collection))},
        self =>
          Db.getRecentNotes(maxRecentNotes)
          |> selectFirstNote
          |> Repromise.wait(((notes, selectedNoteId)) => {
               self.send(NotebookSelected(notes, selectedNoteId));

               switch (selectedNoteId) {
               | None => ()
               | Some(id) => self.send(SelectNote(id))
               };
             }),
      )
    | SelectNotebook(NoteCollection.Notebook(notebook)) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, selectedCollection: Some(notebook.id)},
        self =>
          getNotes(notebook.id)
          |> Repromise.wait(((notes, selectedNoteId)) => {
               self.send(NotebookSelected(notes, selectedNoteId));

               switch (selectedNoteId) {
               | None => ()
               | Some(id) => self.send(SelectNote(id))
               };
             }),
      )
    | NotebookSelected(notes, selectedNoteId) =>
      ReasonReact.Update({...state, notes, contentBlocks: [], selectedNote: selectedNoteId})
    | CreateNotebook(notebook) =>
      let updatedNotebooks = Belt.List.add(state.notebooks, (notebook, 0));
      let newState = {...state, notebooks: updatedNotebooks};

      ReasonReact.UpdateWithSideEffects(
        newState,
        self =>
          Db.withNotification(() => Notebooks.create(notebook))
          |> Promises.mapOk(NoteCollection.fromNotebook)
          |> Promises.mapOk(collection => self.send(SelectNotebook(collection)))
          |> ignore,
      );
    | DeleteNotebook(notebook) =>
      let updatedNotebooks =
        Belt.List.keep(state.notebooks, ((existingNotebook, _noteCount)) =>
          existingNotebook.id != notebook.id
        );

      let newState = {...state, notebooks: updatedNotebooks};
      ReasonReact.UpdateWithSideEffects(
        newState,
        _self => Db.withNotification(() => Notebooks.delete(notebook.id, ())) |> ignore,
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
        _self => Notebooks.update(notebook, ()) |> ignore,
      );
    | SelectNote(noteId) =>
      ReasonReact.UpdateWithSideEffects(
        {...state, selectedNote: Some(noteId)},
        self =>
          ContentBlocks.fromNote(noteId)
          |> Repromise.wait(contentBlocks => self.send(NoteSelected(noteId, contentBlocks))),
      )
    | NoteSelected(noteId, contentBlocks) =>
      ReasonReact.Update({...state, selectedNote: Some(noteId), contentBlocks})
    | CreateNote =>
      ReasonReact.SideEffects(
        self =>
          Notes.create(self.state.selectedCollection |> Belt.Option.getExn)
          |> Promises.tapOk(((note: Data.note, _contentBlock)) => {
               AppState.setSelected(self.state.selectedCollection, Some(note.id));

               fetchAllData() |> Repromise.wait(state => self.send(LoadState(state)));
             })
          |> ignore,
      )
    | DeleteNote(note) =>
      let updatedNotes = Belt.List.keep(state.notes, existingNote => existingNote.id != note.id);

      let newState = {...state, notes: updatedNotes};
      ReasonReact.UpdateWithSideEffects(
        newState,
        _self => Db.withNotification(() => Notes.delete(note.id, ()) |> ignore),
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
        _self => debouncedUpdateContentBlockInDb(updatedContentBlock),
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
        _self => ContentBlocks.update(updatedBlock, ()) |> ignore,
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
        _self => Notes.update(updatedNote, ()) |> ignore,
      );
    },
  didMount: self => {
    let serverSync = () => {
      DbSync.run();
      lastDataRefresh := Some(Utils.now());
    };

    serverSync();
    DataSyncRetry.getPendingChanges() |> Repromise.wait(DataSync.start);

    let loadStateFromDb = () => self.send(ReloadState);
    loadStateFromDb();
    Db.subscribe(loadStateFromDb);

    let removePageVisibleListener =
      Utils.onPageVisible(() =>
        switch (lastDataRefresh^) {
        | Some(date) when Utils.timeElapsedSince(date) > refreshDataMinElapsed => serverSync()
        | _ => ()
        }
      );

    self.onUnmount(() => {
      Db.unsubscribe(loadStateFromDb);
      removePageVisibleListener();
    });
  },
  didUpdate: ({oldSelf: _oldSelf, newSelf}) =>
    AppState.setSelected(newSelf.state.selectedCollection, newSelf.state.selectedNote),
  render: self =>
    <>
      {switch (self.state.initialStateLoaded) {
       | false => ReasonReact.null
       | true => children(self.state, self.send)
       }}
    </>,
};
