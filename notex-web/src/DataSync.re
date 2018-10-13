open Belt;

type changeValue =
  | ContentBlockCreated(Data.contentBlock)
  | ContentBlock(Data.contentBlock)
  | NoteCreated(Data.note)
  | NoteUpdated(Data.note)
  | NotebookCreated(Data.notebook)
  | NotebookUpdated(Data.notebook)
  | NotebookDeleted(string)
  | NoteDeleted(string);

type change = {
  id: string,
  change: changeValue,
};

let pendingChanges: ref(list(change)) = ref([]);

let removePendingChange = change =>
  pendingChanges :=
    List.keep(pendingChanges^, pendingChange => pendingChange !== change);

let syncChange = change =>
  switch (change.change) {
  | ContentBlock(contentBlock) => Api.updateContentBlock(contentBlock)
  | NoteCreated(note) => Api.createNote(note)
  | NoteUpdated(note) => Api.updateNote(note)
  | ContentBlockCreated(contentBlock) => Api.createContentBlock(contentBlock)
  | NotebookCreated(notebook) => Api.createNotebook(notebook)
  | NotebookUpdated(notebook) => Api.updateNotebook(notebook)
  | NotebookDeleted(notebookId) => Api.deleteNotebook(notebookId)
  | NoteDeleted(noteId) => Api.deleteNote(noteId)
  };

let pushChange = change =>
  pendingChanges :=
    (pendingChanges^)
    ->List.keep(pendingChange => pendingChange.id != change.id)
    ->List.concat([change]);

let pushContentBlock = (contentBlock: Data.contentBlock) => {
  let id = "contentBlock:" ++ contentBlock.id;
  let change = {id, change: ContentBlock(contentBlock)};

  pushChange(change);
};

let pushNewContentBlock = (contentBlock: Data.contentBlock) => {
  let id = "contentBlock:created:" ++ contentBlock.id;
  let change = {id, change: ContentBlockCreated(contentBlock)};

  pushChange(change);
};

let pushNewNote = (note: Data.note) => {
  let id = "note:craeted:" ++ note.id;
  let change = {id, change: NoteCreated(note)};

  pushChange(change);
};

let pushNoteChange = (note: Data.note) => {
  let id = "note:updated:" ++ note.id;
  let change = {id, change: NoteUpdated(note)};

  pushChange(change);
};

let pushNewNotebook = (notebook: Data.notebook) => {
  let id = "notebook:craeted:" ++ notebook.id;
  let change = {id, change: NotebookCreated(notebook)};

  pushChange(change);
};

let pushNotebookChange = (notebook: Data.notebook) => {
  let id = "notebook:updated:" ++ notebook.id;
  let change = {id, change: NotebookUpdated(notebook)};

  pushChange(change);
};

let pushNotebookDelete = (notebookId: string) => {
  let id = "notebook:deleted:" ++ notebookId;
  let change = {id, change: NotebookDeleted(notebookId)};

  pushChange(change);
};

let pushNoteDelete = (noteId: string) => {
  let id = "note:deleted:" ++ noteId;
  let change = {id, change: NoteDeleted(noteId)};

  pushChange(change);
};

let rec syncPendingChanges = onComplete => {
  let nextChange = List.take(pendingChanges^, 1);

  switch (nextChange) {
  | Some([change]) =>
    syncChange(change)
    ->Future.get(result => {
        if (Belt.Result.isOk(result)) {
          removePendingChange(change);
        } else {
          pushChange(change);
        };

        syncPendingChanges(onComplete);
      })
  | _ => onComplete()
  };
};

let start = () => {
  let rec onComplete = () =>
    Js.Global.setTimeout(() => syncPendingChanges(onComplete), 3000)
    |> ignore;

  syncPendingChanges(onComplete);
};
