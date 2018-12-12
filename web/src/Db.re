open Belt.Result;
type result('a) = Belt.Result.t('a, Js.Promise.error);

type listener = unit => unit;

module JsonCoders = {
  /* Notebooks */
  let decodeNotebook = json: Data.notebook =>
    Json.Decode.{
      id: json |> field("id", string),
      title: json |> field("title", string),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
      revision: json |> optional(field("revision", string)),
    };

  let encodeNotebook = (notebook: Data.notebook) =>
    Json.Encode.(
      object_([
        ("id", string(notebook.id)),
        ("title", string(notebook.title)),
        ("createdAt", date(notebook.createdAt)),
        ("updatedAt", date(notebook.updatedAt)),
        ("revision", nullable(string, notebook.revision)),
      ])
    );

  /* Notes */
  let decodeNote = json: Data.note =>
    Json.Decode.{
      id: json |> field("id", string),
      notebookId: json |> field("notebookId", string),
      title: json |> field("title", string),
      tags: json |> field("tags", list(string)),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
      revision: json |> optional(field("revision", string)),
    };

  let encodeNote = (note: Data.note) =>
    Json.Encode.(
      object_([
        ("id", string(note.id)),
        ("notebookId", string(note.notebookId)),
        ("title", string(note.title)),
        ("tags", jsonArray(note.tags |> List.map(string) |> Array.of_list)),
        ("createdAt", date(note.createdAt)),
        ("updatedAt", date(note.updatedAt)),
        ("revision", nullable(string, note.revision)),
      ])
    );

  /* ContentBlocks */
  let decodeContentBlock = json: Data.contentBlock => {
    let textContent = json => {
      let text = json |> Json.Decode.field("text", Json.Decode.string);
      Data.TextContent(RichText.fromString(text));
    };

    let codeContent = json => {
      let code = json |> Json.Decode.field("code", Json.Decode.string);
      let language =
        json |> Json.Decode.field("language", Json.Decode.string);

      Data.CodeContent(code, language);
    };

    let content = json => {
      let type_ = json |> Json.Decode.field("type", Json.Decode.string);

      switch (type_) {
      | "text" => json |> Json.Decode.field("data", textContent)
      | "code" => json |> Json.Decode.field("data", codeContent)
      | _other => raise(Not_found)
      };
    };

    Json.Decode.{
      id: json |> field("id", string),
      noteId: json |> field("noteId", string),
      content: json |> field("content", content),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
      revision: json |> optional(field("revision", string)),
    };
  };

  let encodeContentBlock = (contentBlock: Data.contentBlock) => {
    let type_ = content =>
      switch (content) {
      | Data.TextContent(_text) => "text"
      | Data.CodeContent(_code, _language) => "code"
      };

    let data = content =>
      switch (content) {
      | Data.TextContent(richText) =>
        Json.Encode.(
          object_([("text", string(RichText.toString(richText)))])
        )
      | Data.CodeContent(code, language) =>
        Json.Encode.(
          object_([
            ("code", string(code)),
            ("language", string(language)),
          ])
        )
      };

    let content = content =>
      Json.Encode.(
        object_([
          ("type", string(type_(content))),
          ("data", data(content)),
        ])
      );

    Json.Encode.(
      object_([
        ("id", string(contentBlock.id)),
        ("noteId", string(contentBlock.noteId)),
        ("content", content(contentBlock.content)),
        ("createdAt", date(contentBlock.createdAt)),
        ("updatedAt", date(contentBlock.updatedAt)),
        ("revision", nullable(string, contentBlock.revision)),
      ])
    );
  };
};

let notebooksStore = "notebooks";
let notesStore = "notes";
let contentBlocksStore = "contentBlocks";

let iDb: ref(option(IndexedDB.DB.t)) = ref(None);
let initDb = () =>
  IndexedDB.(
    open_("pragma", 1, upgradeDb =>
      switch (UpgradeDb.oldVersion(upgradeDb)) {
      | version when version <= 1 =>
        UpgradeDb.createObjectStore(
          upgradeDb,
          notebooksStore,
          ObjectStoreParams.make(~keyPath="id"),
        )
        |> ignore;

        UpgradeDb.createObjectStore(
          upgradeDb,
          notesStore,
          ObjectStoreParams.make(~keyPath="id"),
        )
        |> ignore;

        let tx = UpgradeDb.transaction(upgradeDb);

        Transaction.objectStore(tx, notesStore)
        ->ObjectStore.createIndex(
            "forNotebook",
            "notebookId",
            CreateIndexParams.make(~unique=false, ~multiEntry=false),
          )
        |> ignore;

        UpgradeDb.createObjectStore(
          upgradeDb,
          contentBlocksStore,
          ObjectStoreParams.make(~keyPath="id"),
        )
        |> ignore;

        Transaction.objectStore(tx, contentBlocksStore)
        ->ObjectStore.createIndex(
            "forNote",
            "noteId",
            CreateIndexParams.make(~unique=false, ~multiEntry=false),
          )
        |> ignore;
      | _ => () /* No upgrade needed */
      }
    )
  )
  |> Promises.toResultPromise;

let dbPromise = () =>
  switch (iDb^) {
  | None =>
    initDb()
    |> Repromise.map(result => {
         let db = Belt.Result.getExn(result);

         iDb := Some(db);
         db;
       })
  | Some(db) => Repromise.resolved(db)
  };

let toOptionPromise = (jsPromise: Js.Promise.t(option('a))) =>
  jsPromise
  |> Repromise.Rejectable.fromJsPromise
  |> Repromise.Rejectable.map(
       fun
       | None => None
       | Some(value) => Some(value),
     )
  |> Repromise.Rejectable.catch(error => {
       Js.Console.error2("Db promise failed: ", error);
       Repromise.resolved(None);
     });

let awaitTransactionPromise = (transaction: IndexedDB.Transaction.t) =>
  IndexedDB.Transaction.complete(transaction)->Promises.toResultPromise;

let listeners: ref(list(listener)) = ref([]);

let subscribe = listener => listeners := [listener, ...listeners^];

let unsubscribe = listener =>
  listeners := Belt.List.keep(listeners^, l => l !== listener);

let getNotes = (notebookId: string) =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, notesStore, Transaction.ReadOnly)
         ->Transaction.objectStore(notesStore)
         ->ObjectStore.index("forNotebook")
         ->IndexedDB.Index.getAllByKey(notebookId)
         ->Promises.toResultPromise
         |> Repromise.map(
              fun
              | Ok(array) =>
                array
                ->Belt.List.fromArray
                ->Belt.List.map(JsonCoders.decodeNote)
              | Error(_) => [],
            )
       )
     );

let countNotes = (notebookId: string) =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, notesStore, Transaction.ReadOnly)
         ->Transaction.objectStore(notesStore)
         ->ObjectStore.index("forNotebook")
         ->IndexedDB.Index.countByKey(notebookId)
         ->Promises.toResultPromise
         |> Repromise.map(Belt.Result.getWithDefault(_, 0))
       )
     );

let getNote = (noteId: string) =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, notesStore, Transaction.ReadOnly)
         ->Transaction.objectStore(notesStore)
         ->ObjectStore.get(noteId)
         ->toOptionPromise
         |> Repromise.map(v => Belt.Option.map(v, JsonCoders.decodeNote))
       )
     );

let getNotebooks = () =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, notebooksStore, Transaction.ReadOnly)
         ->Transaction.objectStore(notebooksStore)
         ->ObjectStore.getAll
         ->Promises.toResultPromise
         |> Repromise.andThen(
              fun
              | Ok(array) =>
                array
                ->Belt.List.fromArray
                ->Belt.List.map(JsonCoders.decodeNotebook)
                ->Belt.List.map(n =>
                    countNotes(n.id) |> Repromise.map(count => (n, count))
                  )
                ->Repromise.all
              | Error(_) => Repromise.resolved([]),
            )
       )
     );

let getNotebook = (notebookId: string) =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, notebooksStore, Transaction.ReadOnly)
         ->Transaction.objectStore(notebooksStore)
         ->ObjectStore.get(notebookId)
         ->toOptionPromise
         |> Repromise.map(v => Belt.Option.map(v, JsonCoders.decodeNotebook))
       )
     );

let getContentBlocks = noteId =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, contentBlocksStore, Transaction.ReadOnly)
         ->Transaction.objectStore(contentBlocksStore)
         ->ObjectStore.index("forNote")
         ->IndexedDB.Index.getAllByKey(noteId)
         ->Promises.toResultPromise
         |> Repromise.map(
              fun
              | Ok(array) =>
                array
                ->Belt.List.fromArray
                ->Belt.List.map(JsonCoders.decodeContentBlock)
              | Error(_) => [],
            )
       )
     );

let getContentBlock = blockId =>
  dbPromise()
  |> Repromise.andThen(db =>
       IndexedDB.(
         DB.transaction(db, contentBlocksStore, Transaction.ReadOnly)
         ->Transaction.objectStore(contentBlocksStore)
         ->ObjectStore.get(blockId)
         ->toOptionPromise
         |> Repromise.map(v =>
              Belt.Option.map(v, JsonCoders.decodeContentBlock)
            )
       )
     );

let addNotebook = notebook =>
  IndexedDB.(
    dbPromise()
    |> Repromise.andThen(db => {
         let tx = DB.transaction(db, notebooksStore, Transaction.ReadWrite);
         let data = JsonCoders.encodeNotebook(notebook);

         Transaction.objectStore(tx, notebooksStore)
         ->ObjectStore.put(data)
         ->ignore;

         awaitTransactionPromise(tx);
       })
  );

let addNote = note =>
  IndexedDB.(
    dbPromise()
    |> Repromise.andThen(db => {
         let tx = DB.transaction(db, notesStore, Transaction.ReadWrite);
         let data = JsonCoders.encodeNote(note);

         Transaction.objectStore(tx, notesStore)
         ->ObjectStore.put(data)
         ->ignore;

         awaitTransactionPromise(tx);
       })
  );

let addContentBlock = block =>
  IndexedDB.(
    dbPromise()
    |> Repromise.andThen(db => {
         let tx =
           DB.transaction(db, contentBlocksStore, Transaction.ReadWrite);
         let data = JsonCoders.encodeContentBlock(block);

         Transaction.objectStore(tx, contentBlocksStore)
         ->ObjectStore.put(data)
         ->ignore;

         awaitTransactionPromise(tx);
       })
  );

let createNote = (notebookId: string) => {
  let now = Js.Date.fromFloat(Js.Date.now());

  let note: Data.note = {
    id: Utils.generateId(),
    notebookId,
    title: "Untitled note",
    tags: [],
    createdAt: now,
    updatedAt: now,
    revision: None,
  };

  let contentBlock: Data.contentBlock = {
    id: Utils.generateId(),
    noteId: note.id,
    content: Data.TextContent(RichText.create()),
    createdAt: now,
    updatedAt: now,
    revision: None,
  };

  Repromise.all([addNote(note), addContentBlock(contentBlock)])
  |> Repromise.map(_ => {
       /* FIXME: only push when succesful */
       DataSync.pushNewNote(note);
       DataSync.pushNewContentBlock(contentBlock);

       Ok((note, contentBlock));
     });
};

let createNotebook = notebook =>
  addNotebook(notebook)
  |> Repromise.map(_result => {
       /* FIXME: only push when create is succesfull */
       DataSync.pushNewNotebook(notebook) |> ignore;
       Ok(notebook);
     });

let updateContentBlock = (contentBlock: Data.contentBlock, ~sync=true, ()) =>
  addContentBlock(contentBlock)
  |> Repromise.map(result => {
       /* FIXME: only push when create is succesfull */
       if (sync) {
         DataSync.pushContentBlock(contentBlock);
       };

       result;
     });

let updateNote = (note: Data.note, ~sync=true, ()) =>
  addNote(note)
  |> Repromise.map(result => {
       /* FIXME: only push when create is succesfull */
       if (sync) {
         DataSync.pushNoteChange(note);
       };

       result;
     });

let updateNotebook = (notebook: Data.notebook, ~sync=true, ()) =>
  addNotebook(notebook)
  |> Repromise.map(result => {
       /* FIXME: only push when create is succesfull */
       if (sync) {
         DataSync.pushNotebookChange(notebook);
       };

       result;
     });

let deleteNotebook = (notebookId: string, ~sync=true, ()) =>
  dbPromise()
  |> Repromise.andThen(db => {
       open IndexedDB;
       let tx = DB.transaction(db, notebooksStore, Transaction.ReadWrite);

       Transaction.objectStore(tx, notebooksStore)
       ->ObjectStore.delete(notebookId)
       ->Promises.toResultPromise;
     })
  |> Repromise.map(_result => {
       /* FIXME: only push when succesfull */
       if (sync) {
         DataSync.pushNotebookDelete(notebookId);
       };

       Ok();
     });

let deleteNote = (noteId: string, ~sync=true, ()) =>
  dbPromise()
  |> Repromise.andThen(db => {
       open IndexedDB;
       let tx = DB.transaction(db, notesStore, Transaction.ReadWrite);

       Transaction.objectStore(tx, notesStore)
       ->ObjectStore.delete(noteId)
       ->Promises.toResultPromise;
     })
  |> Repromise.map(_result => {
       /* FIXME: only push when succesfull */
       if (sync) {
         DataSync.pushNotebookDelete(noteId);
       };

       Ok();
     });

let deleteContentBlock = (contentBlockId: string) =>
  dbPromise()
  |> Repromise.andThen(db => {
       open IndexedDB;
       let tx = DB.transaction(db, contentBlocksStore, Transaction.ReadWrite);

       Transaction.objectStore(tx, contentBlocksStore)
       ->ObjectStore.delete(contentBlockId)
       ->Promises.toResultPromise;
     })
  |> Repromise.map(_result
       /* FIXME: only push when succesfull */
       => Ok());

let insertRevision = (revision: string) =>
  LocalStorage.setItem("pragma-revision", revision)->Repromise.resolved;

let getRevision = () =>
  LocalStorage.getItem("pragma-revision")->Repromise.resolved;

let withNotification = fn => {
  let result = fn();
  Belt.List.forEach(listeners^, l => l());

  result;
};

let withPromiseNotification = promise =>
  promise |> Repromise.wait(_ => Belt.List.forEach(listeners^, l => l()));

let clear = () => {
  IndexedDB.delete("pragma") |> ignore;
  LocalStorage.clear();
};
