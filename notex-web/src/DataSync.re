type changeValue =
  | ContentBlockCreated(Data.contentBlock)
  | ContentBlock(Data.contentBlock)
  | NoteCreated(Data.note)
  | NoteUpdated(Data.note)
  | NotebookCreated(Data.notebook)
  | NotebookUpdated(Data.notebook);

type change = {
  id: string,
  change: changeValue,
};

let pendingChanges: ref(list(change)) = ref([]);

let pushChange = change =>
  pendingChanges :=
    (pendingChanges^)
    ->Belt.List.keep(pendingChange => pendingChange.id != change.id)
    ->Belt.List.concat([change]);

let removePendingChange = change =>
  pendingChanges :=
    Belt.List.keep(pendingChanges^, pendingChange => pendingChange !== change);

/* FIXME: this basic sync queue mechanism has a few caveats:
   1. It's not persistent.
   2. If requests take > 10 seconds, they will start overlapping.
   3. If the queue looks like this: [create:123, update:123] and the create operartion would
      fail, the update would still be attempted. This is usually not a problem, but if it happens
      when there's a ID collision this could lead to data loss.
   */
let start = () =>
  Js.Global.setInterval(
    () =>
      Belt.List.forEach(
        pendingChanges^,
        change => {
          let result =
            switch (change.change) {
            | ContentBlock(contentBlock) =>
              Api.updateContentBlock(contentBlock)
            | NoteCreated(note) => Api.createNote(note)
            | NoteUpdated(note) => Api.updateNote(note)
            | ContentBlockCreated(contentBlock) =>
              Api.createContentBlock(contentBlock)
            | NotebookCreated(notebook) => Api.createNotebook(notebook)
            | NotebookUpdated(notebook) => Api.updateNotebook(notebook)
            };

          Future.get(result, result =>
            if (Belt.Result.isOk(result)) {
              removePendingChange(change);
            }
          );
        },
      ),
    10 * 1000,
  );

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
