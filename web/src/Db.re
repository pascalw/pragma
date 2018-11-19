type state = {
  notebooks: list(Data.notebook),
  notes: list(Data.note),
  contentBlocks: list(Data.contentBlock),
};

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
let noteKey = noteId => "pragma-note:" ++ noteId;
let blockKey = blockId => "pragma-block:" ++ blockId;

let keyToId = key => Js.String.split(":", key)->Belt.Array.getExn(1);

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
    noteKey(note.id),
    JsonCoders.encodeNote(note) |> Json.stringify,
  );

let getStoredNote = noteId =>
  LocalStorage.getItem(noteKey(noteId))
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeNote
    );

let storeContentBlock = (contentBlock: Data.contentBlock) =>
  LocalStorage.setItem(
    blockKey(contentBlock.id),
    JsonCoders.encodeContentBlock(contentBlock) |> Json.stringify,
  );

let getStoredContentBlock = blockId =>
  LocalStorage.getItem(blockKey(blockId))
  ->Belt.Option.map(jsonString =>
      Json.parseOrRaise(jsonString)->JsonCoders.decodeContentBlock
    );

let localStorageKeys = LocalStorage.keys();

let getStoredNotebooks = () =>
  localStorageKeys
  ->Belt.List.keep(key => Js.String.startsWith(notebookKey(""), key))
  ->Belt.List.map(key => {
      let id = keyToId(key);
      getStoredNotebook(id) |> Belt.Option.getExn;
    });

let getStoredNotes = () =>
  localStorageKeys
  ->Belt.List.keep(key => Js.String.startsWith(noteKey(""), key))
  ->Belt.List.map(key => {
      let id = keyToId(key);
      getStoredNote(id) |> Belt.Option.getExn;
    });

let getStoredContentBlocks = () =>
  localStorageKeys
  ->Belt.List.keep(key => Js.String.startsWith(blockKey(""), key))
  ->Belt.List.map(key => {
      let id = keyToId(key);
      getStoredContentBlock(id) |> Belt.Option.getExn;
    });

let initialState = {
  notebooks: getStoredNotebooks(),
  notes: getStoredNotes(),
  contentBlocks: getStoredContentBlocks(),
};

let state = ref(initialState);

let listeners: ref(list(listener)) = ref([]);

let getState = () => Future.value(state^);

let saveState = newState => {
  state := newState;
  Future.value();
};

let clear = () => saveState(initialState) |> ignore;

let subscribe = listener => listeners := [listener, ...listeners^];

let unsubscribe = listener =>
  listeners := Belt.List.keep(listeners^, l => l !== listener);

let getNotes_ = (state, notebookId) =>
  Belt.List.keep(state.notes, note => note.notebookId == notebookId);

let getNotes = notebookId =>
  Future.map(getState(), state => getNotes_(state, notebookId));

let getNote = noteId =>
  Future.map(getState(), state =>
    Belt.List.keep(state.notes, n => n.id == noteId)->Belt.List.head
  );

let getNotebooks = () =>
  Future.map(getState(), state =>
    state.notebooks
    |> List.map((n: Data.notebook) =>
         (n, getNotes_(state, n.id)->List.length)
       )
  );

let getNotebook = notebookId =>
  Future.map(getState(), state =>
    Belt.List.keep(state.notebooks, n => n.id == notebookId)->Belt.List.head
  );

let getContentBlocks = noteId =>
  Future.map(getState(), state =>
    state.contentBlocks->(Belt.List.keep(cb => cb.noteId == noteId))
  );

let getContentBlock = contentBlockId =>
  Future.map(getState(), state =>
    state.contentBlocks
    ->(Belt.List.keep(cb => cb.id == contentBlockId))
    ->Belt.List.head
  );

let addNotebook = notebook =>
  Future.map(
    getState(),
    state => {
      let newState = {
        ...state,
        notebooks: state.notebooks->(Belt.List.add(notebook)),
      };

      storeNotebook(notebook);
      saveState(newState) |> ignore;
    },
  );

let addNote = note =>
  Future.map(
    getState(),
    state => {
      let newState = {...state, notes: state.notes->(Belt.List.add(note))};

      storeNote(note);
      saveState(newState) |> ignore;
    },
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

  getState()
  ->Future.flatMap(state => {
      let newState = {
        ...state,
        notes: state.notes @ [note],
        contentBlocks: state.contentBlocks @ [contentBlock],
      };

      storeNote(note);
      storeContentBlock(contentBlock);

      saveState(newState);
    })
  ->Future.tap(_ => {
      DataSync.pushNewNote(note);
      DataSync.pushNewContentBlock(contentBlock);
    })
  ->Future.flatMap(_ => Future.value((note, contentBlock)));
};

let createNotebook = notebook =>
  getState()
  ->Future.map(state =>
      {...state, notebooks: state.notebooks->Belt.List.add(notebook)}
    )
  ->Future.flatMap(saveState)
  ->Future.tap(_ => storeNotebook(notebook))
  ->Future.tap(_ => DataSync.pushNewNotebook(notebook))
  ->Future.flatMap(_ => Future.value(notebook));

let addContentBlock = contentBlock =>
  Future.map(
    getState(),
    state => {
      let newState = {
        ...state,
        contentBlocks: state.contentBlocks->Belt.List.add(contentBlock),
      };

      storeContentBlock(contentBlock);
      saveState(newState) |> ignore;
    },
  );

let updateContentBlock = (contentBlock: Data.contentBlock, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedContentBlocks: list(Data.contentBlock) =
        Belt.List.map(state.contentBlocks, block =>
          if (block.id == contentBlock.id) {
            contentBlock;
          } else {
            block;
          }
        );

      {...state, contentBlocks: updatedContentBlocks};
    })
  ->Future.tap(_ => storeContentBlock(contentBlock))
  ->Future.tap(_ => sync ? DataSync.pushContentBlock(contentBlock) : ())
  ->Future.flatMap(saveState);

let updateNote = (note: Data.note, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedNotes: list(Data.note) =
        Belt.List.map(state.notes, existingNote =>
          if (existingNote.id == note.id) {
            note;
          } else {
            existingNote;
          }
        );

      {...state, notes: updatedNotes};
    })
  ->Future.tap(_ => storeNote(note))
  ->Future.tap(_ => sync ? DataSync.pushNoteChange(note) : ())
  ->Future.flatMap(saveState);

let updateNotebook = (notebook: Data.notebook, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedNotebooks: list(Data.notebook) =
        Belt.List.map(state.notebooks, existingNotebook =>
          if (existingNotebook.id == notebook.id) {
            notebook;
          } else {
            existingNotebook;
          }
        );

      {...state, notebooks: updatedNotebooks};
    })
  ->Future.tap(_ => storeNotebook(notebook))
  ->Future.tap(_ => sync ? DataSync.pushNotebookChange(notebook) : ())
  ->Future.flatMap(saveState);

let deleteNotebook = (notebookId: string, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedNotebooks =
        Belt.List.keep(state.notebooks, existingNotebook =>
          existingNotebook.id != notebookId
        );

      {...state, notebooks: updatedNotebooks};
    })
  ->Future.tap(_ => LocalStorage.removeItem(notebookKey(notebookId)))
  ->Future.tap(_ => sync ? DataSync.pushNotebookDelete(notebookId) : ())
  ->Future.flatMap(saveState);

let deleteNote = (noteId: string, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedNotes =
        Belt.List.keep(state.notes, existingNote => existingNote.id != noteId);

      {...state, notes: updatedNotes};
    })
  ->Future.tap(_ => LocalStorage.removeItem(noteKey(noteId)))
  ->Future.tap(_ => sync ? DataSync.pushNoteDelete(noteId) : ())
  ->Future.flatMap(saveState);

let deleteContentBlock = (contentBlockId: string, ~sync=true, ()) =>
  getState()
  ->Future.map(state => {
      let updatedContentBlocks =
        Belt.List.keep(state.contentBlocks, existingBlock =>
          existingBlock.id != contentBlockId
        );

      {...state, contentBlocks: updatedContentBlocks};
    })
  ->Future.tap(_ => LocalStorage.removeItem(blockKey(contentBlockId)))
  ->Future.tap(_ => sync ? DataSync.pushContentBlockDelete(contentBlockId) : ())
  ->Future.flatMap(saveState);

let insertRevision = (revision: string) =>
  LocalStorage.setItem("pragma-revision", revision)->Future.value;

let getRevision = () => LocalStorage.getItem("pragma-revision")->Future.value;

let withNotification = fn => {
  let result = fn();
  Belt.List.forEach(listeners^, l => l());

  result;
};
