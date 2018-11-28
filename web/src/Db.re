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

let notebookKey = notebookId => "pragma-notebook:" ++ notebookId;
let noteKey = (noteId, notebookId) =>
  "pragma-note:" ++ noteId ++ ":" ++ notebookId;
let blockKey = (blockId, noteId) =>
  "pragma-block:" ++ blockId ++ ":" ++ noteId;

let keyToId = key => Js.String.split(":", key)->Belt.Array.getExn(1);

let findBlockKey = blockId =>
  LocalStorage.keys()
  ->Belt.List.getBy(key =>
      Js.String.startsWith("pragma-block:" ++ blockId, key)
    );

let findNoteKey = noteId =>
  LocalStorage.keys()
  ->Belt.List.getBy(key =>
      Js.String.startsWith("pragma-note:" ++ noteId, key)
    );

let findBlockKeysForNote = noteId => {
  let re = Js.Re.fromString("pragma-block:[^:]*:" ++ noteId);

  LocalStorage.keys()
  ->Belt.List.keep(key => Js.Re.exec(key, re) |> Belt.Option.isSome);
};

let findNoteKeysForNotebook = notebookId => {
  let re = Js.Re.fromString("pragma-note:[^:]*:" ++ notebookId);

  LocalStorage.keys()
  ->Belt.List.keep(key => Js.Re.exec(key, re) |> Belt.Option.isSome);
};

let storeNotebook = (notebook: Data.notebook) =>
  LocalStorage.setItem(
    notebookKey(notebook.id),
    JsonCoders.encodeNotebook(notebook) |> Json.stringify,
  );

let getStoredNotebook = notebookId =>
  LocalStorage.getItem(notebookKey(notebookId))
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeNotebook
    );

let storeNote = (note: Data.note) =>
  LocalStorage.setItem(
    noteKey(note.id, note.notebookId),
    JsonCoders.encodeNote(note) |> Json.stringify,
  );

let getStoredNoteByKey = key =>
  LocalStorage.getItem(key)
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeNote
    );

let getStoredNote = noteId =>
  findNoteKey(noteId)
  ->Belt.Option.flatMap(LocalStorage.getItem)
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeNote
    );

let storeContentBlock = (contentBlock: Data.contentBlock) =>
  LocalStorage.setItem(
    blockKey(contentBlock.id, contentBlock.noteId),
    JsonCoders.encodeContentBlock(contentBlock) |> Json.stringify,
  );

let getStoredContentBlockByKey = key =>
  LocalStorage.getItem(key)
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeContentBlock
    );

let getStoredContentBlock = blockId =>
  findBlockKey(blockId)->Belt.Option.flatMap(getStoredContentBlockByKey);

let getStoredNotebooks = () =>
  LocalStorage.keys()
  ->Belt.List.keep(key => Js.String.startsWith(notebookKey(""), key))
  ->Belt.List.map(key => {
      let id = keyToId(key);
      getStoredNotebook(id) |> Belt.Option.getExn;
    });

let listeners: ref(list(listener)) = ref([]);

let subscribe = listener => listeners := [listener, ...listeners^];

let unsubscribe = listener =>
  listeners := Belt.List.keep(listeners^, l => l !== listener);

let getNotes' = notebookId =>
  findNoteKeysForNotebook(notebookId)
  ->Belt.List.map(key => getStoredNoteByKey(key) |> Belt.Option.getExn);

let getNotes = notebookId => getNotes'(notebookId)->Future.value;

let getNote = noteId => getStoredNote(noteId)->Future.value;

let getNotebooks = () =>
  getStoredNotebooks()
  |> List.map((notebook: Data.notebook) =>
       (notebook, getNotes'(notebook.id)->List.length)
     )
  |> Future.value;

let getNotebook = notebookId => getStoredNotebook(notebookId)->Future.value;

let getContentBlocks = noteId =>
  findBlockKeysForNote(noteId)
  ->Belt.List.map(key =>
      getStoredContentBlockByKey(key) |> Belt.Option.getExn
    )
  ->Future.value;

let getContentBlock = blockId => getStoredContentBlock(blockId)->Future.value;

let addNotebook = notebook => {
  storeNotebook(notebook);
  Future.value();
};

let addNote = note => {
  storeNote(note);
  Future.value();
};

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

  storeNote(note);
  storeContentBlock(contentBlock);

  DataSync.pushNewNote(note);
  DataSync.pushNewContentBlock(contentBlock);

  Future.value((note, contentBlock));
};

let createNotebook = notebook => {
  storeNotebook(notebook);
  DataSync.pushNewNotebook(notebook);
  Future.value(notebook);
};

let addContentBlock = block => {
  storeContentBlock(block);
  Future.value();
};

let updateContentBlock = (contentBlock: Data.contentBlock, ~sync=true, ()) => {
  storeContentBlock(contentBlock);
  if (sync) {
    DataSync.pushContentBlock(contentBlock);
  };
  Future.value();
};

let updateNote = (note: Data.note, ~sync=true, ()) => {
  storeNote(note);
  if (sync) {
    DataSync.pushNoteChange(note);
  };
  Future.value();
};

let updateNotebook = (notebook: Data.notebook, ~sync=true, ()) => {
  storeNotebook(notebook);
  if (sync) {
    DataSync.pushNotebookChange(notebook);
  };
  Future.value();
};

let deleteNotebook = (notebookId: string, ~sync=true, ()) => {
  LocalStorage.removeItem(notebookKey(notebookId));
  if (sync) {
    DataSync.pushNotebookDelete(notebookId);
  };
  Future.value();
};

let deleteNote = (noteId: string, ~sync=true, ()) => {
  switch (findNoteKey(noteId)) {
  | None => ()
  | Some(key) =>
    LocalStorage.removeItem(key);
    if (sync) {
      DataSync.pushNoteDelete(noteId);
    };
  };
  Future.value();
};

let deleteContentBlock = (contentBlockId: string) => {
  switch (findBlockKey(contentBlockId)) {
  | None => ()
  | Some(key) => LocalStorage.removeItem(key)
  };
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
