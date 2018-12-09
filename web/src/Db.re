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

let unwrapResult = promise => {
  FutureJs.fromPromise(promise, Js.String.make)
  ->Future.map(
      fun
      | Belt.Result.Ok(value) => {
        value
      }
      | Belt.Result.Error(error) =>
        Js.Exn.raiseError("IndexedDB result error: " ++ error),
    );
};

let awaitTransaction = (transaction: IndexedDB.Transaction.t) =>
  IndexedDB.Transaction.complete(transaction)
  ->FutureJs.fromPromise(Js.String.make)
  ->Future.map(
      fun
      | Belt.Result.Ok(_) => ()
      | Belt.Result.Error(error) =>
        Js.Exn.raiseError("IndexedDB transaction failed: " ++ error),
    );

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
  ->FutureJs.fromPromise(Js.String.make);

let db = () =>
  switch (iDb^) {
  | None =>
    initDb()
    ->Future.map(
        fun
        | Belt.Result.Ok(db) => {
            iDb := Some(db);
            db;
          }
        | Belt.Result.Error(error) =>
          Js.Exn.raiseError("Failed to open IndexedDB: " ++ error),
      )
  | Some(db) => Future.value(db)
  };


let listeners: ref(list(listener)) = ref([]);

let subscribe = listener => listeners := [listener, ...listeners^];

let unsubscribe = listener =>
  listeners := Belt.List.keep(listeners^, l => l !== listener);

let getNotes = (notebookId: string) => {
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, notesStore, Transaction.ReadOnly)
      ->Transaction.objectStore(notesStore)
      ->ObjectStore.index("forNotebook")
      ->IndexedDB.Index.getAllByKey(notebookId)
      ->unwrapResult
      ->Future.map(Belt.List.fromArray)
      ->Future.map(List.map(JsonCoders.decodeNote))
    });
  });
};

let getNote = (noteId: string) =>
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, notesStore, Transaction.ReadOnly)
      ->Transaction.objectStore(notesStore)
      ->ObjectStore.get(noteId)
      ->unwrapResult
      ->Future.map(v => Belt.Option.map(v, JsonCoders.decodeNote))
    });
  });

let getNotebooks = () =>
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, notebooksStore, Transaction.ReadOnly)
      ->Transaction.objectStore(notebooksStore)
      ->ObjectStore.getAll
      ->unwrapResult
      ->Future.map(Belt.List.fromArray)
      ->Future.map(List.map(n => (JsonCoders.decodeNotebook(n), 0))) /* FIXME: */
    });
  });

let getNotebook = (notebookId: string) => 
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, notebooksStore, Transaction.ReadOnly)
      ->Transaction.objectStore(notebooksStore)
      ->ObjectStore.get(notebookId)
      ->unwrapResult
      ->Future.map(v => Belt.Option.map(v, JsonCoders.decodeNotebook))
    });
  });

let getContentBlocks = noteId =>
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, contentBlocksStore, Transaction.ReadOnly)
      ->Transaction.objectStore(contentBlocksStore)
      ->ObjectStore.index("forNote")
      ->IndexedDB.Index.getAllByKey(noteId)
      ->unwrapResult
      ->Future.map(Belt.List.fromArray)
      ->Future.map(List.map(JsonCoders.decodeContentBlock))
    });
  });

let getContentBlock = blockId => 
  db()
  ->Future.flatMap(db => {
    IndexedDB.({
      DB.transaction(db, contentBlocksStore, Transaction.ReadOnly)
      ->Transaction.objectStore(contentBlocksStore)
      ->ObjectStore.get(blockId)
      ->unwrapResult
      ->Future.map(v => Belt.Option.map(v, JsonCoders.decodeContentBlock))
    });
  });

let addNotebook = notebook =>
  IndexedDB.(
    db()
    ->Future.flatMap(db => {
        let tx = DB.transaction(db, notebooksStore, Transaction.ReadWrite);
        let data = JsonCoders.encodeNotebook(notebook);

        Transaction.objectStore(tx, notebooksStore)
        ->ObjectStore.put(data)
        ->ignore;

        awaitTransaction(tx);
      })
  );

let addNote = note =>
  IndexedDB.(
    db()
    ->Future.flatMap(db => {
        let tx = DB.transaction(db, notesStore, Transaction.ReadWrite);
        let data = JsonCoders.encodeNote(note);

        Transaction.objectStore(tx, notesStore)
        ->ObjectStore.put(data)
        ->ignore;

        awaitTransaction(tx);
      })
  );

let addContentBlock = block =>
  IndexedDB.(
    db()
    ->Future.flatMap(db => {
        let tx = DB.transaction(db, contentBlocksStore, Transaction.ReadWrite);
        let data = JsonCoders.encodeContentBlock(block);

        Transaction.objectStore(tx, contentBlocksStore)
        ->ObjectStore.put(data)
        ->ignore;

        awaitTransaction(tx);
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

  addNote(note) |> ignore;
  addContentBlock(contentBlock) |> ignore;

  DataSync.pushNewNote(note);
  DataSync.pushNewContentBlock(contentBlock);

  Future.value((note, contentBlock));
};

let createNotebook = notebook => {
  addNotebook(notebook)
  ->Future.map(_ => {
    DataSync.pushNewNotebook(notebook);
  }) |> ignore;

  Future.value(notebook);
};

let updateContentBlock = (contentBlock: Data.contentBlock, ~sync=true, ()) => {
  addContentBlock(contentBlock)
  ->Future.map(_ => {
    if (sync) {
      DataSync.pushContentBlock(contentBlock);
    };
  });
};

let updateNote = (note: Data.note, ~sync=true, ()) => {
  addNote(note)
  ->Future.map(_ => {
    if (sync) {
      DataSync.pushNoteChange(note);
    };
  });
};

let updateNotebook = (notebook: Data.notebook, ~sync=true, ()) => {
  addNotebook(notebook)
  ->Future.map(_ => {
    if (sync) {
      DataSync.pushNotebookChange(notebook);
    };
  });
};

let deleteNotebook = (notebookId: string, ~sync=true, ()) => {
  IndexedDB.({
    db()
    ->Future.get(db => {
      let tx = DB.transaction(db, notebooksStore, Transaction.ReadWrite);

      Transaction.objectStore(tx, notebooksStore)
      ->ObjectStore.delete(notebookId)
      ->ignore;
    });
  });

  if (sync) {
    DataSync.pushNotebookDelete(notebookId);
  };

  Future.value();
};

let deleteNote = (noteId: string, ~sync=true, ()) => {
  IndexedDB.({
    db()
    ->Future.get(db => {
      let tx = DB.transaction(db, notesStore, Transaction.ReadWrite);

      Transaction.objectStore(tx, notesStore)
      ->ObjectStore.delete(noteId)
      ->ignore;
    });
  });

  if (sync) {
    DataSync.pushNoteDelete(noteId);
  };

  Future.value();
};

let deleteContentBlock = (contentBlockId: string) => {
  IndexedDB.({
    db()
    ->Future.get(db => {
      let tx = DB.transaction(db, contentBlocksStore, Transaction.ReadWrite);

      Transaction.objectStore(tx, contentBlocksStore)
      ->ObjectStore.delete(contentBlockId)
      ->ignore;
    });
  });

  Future.value();
};

let insertRevision = (revision: string) =>
  LocalStorage.setItem("pragma-revision", revision)->Future.value;

let getRevision = () => LocalStorage.getItem("pragma-revision")->Future.value;

let withNotification = fn => {
  let result = fn();
  Belt.List.forEach(listeners^, l => l());

  result;
};
