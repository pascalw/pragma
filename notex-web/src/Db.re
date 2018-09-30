type state = {
  revision: option(string),
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
      name: json |> field("name", string),
      createdAt: json |> field("createdAt", date),
      systemUpdatedAt: json |> field("systemUpdatedAt", date),
    };

  let encodeNotebook = (notebook: Data.notebook) =>
    Json.Encode.(
      object_([
        ("id", string(notebook.id)),
        ("name", string(notebook.name)),
        ("createdAt", date(notebook.createdAt)),
        ("systemUpdatedAt", date(notebook.systemUpdatedAt)),
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
      systemUpdatedAt: json |> field("systemUpdatedAt", date),
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
        ("systemUpdatedAt", date(note.systemUpdatedAt)),
      ])
    );

  /* ContentBlocks */
  let decodeContentBlock = json: Data.contentBlock => {
    let textContent = json => {
      let text = json |> Json.Decode.field("text", Json.Decode.string);
      Data.TextContent(text);
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
      | Data.TextContent(text) =>
        Json.Encode.(object_([("text", string(text))]))
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
      ])
    );
  };

  /* State */
  let encodeState = state =>
    Json.Encode.(
      object_([
        (
          "revision",
          switch (state.revision) {
          | Some(revision) => string(revision)
          | None => null
          },
        ),
        (
          "notebooks",
          state.notebooks
          |> List.map(encodeNotebook)
          |> Array.of_list
          |> jsonArray,
        ),
        (
          "notes",
          state.notes |> List.map(encodeNote) |> Array.of_list |> jsonArray,
        ),
        (
          "contentBlocks",
          state.contentBlocks
          |> List.map(encodeContentBlock)
          |> Array.of_list
          |> jsonArray,
        ),
      ])
    );

  let decodeState = json =>
    Json.Decode.{
      revision: json |> optional(field("revision", string)),
      notebooks: json |> field("notebooks", list(decodeNotebook)),
      notes: json |> field("notes", list(decodeNote)),
      contentBlocks:
        json |> field("contentBlocks", list(decodeContentBlock)),
    };
};

let state = ref(None);

let listeners: ref(list(listener)) = ref([]);

let getState = () =>
  switch (state^) {
  | Some(state) => Future.value(state)
  | None =>
    switch (LocalStorage.getItem("notex-state")) {
    | None =>
      Future.value({
        revision: None,
        notebooks: [],
        notes: [],
        contentBlocks: [],
      })
    | Some(value) =>
      let json = Json.parseOrRaise(value);
      let savedState = json |> JsonCoders.decodeState;

      state := Some(savedState);
      Future.value(savedState);
    }
  };

let saveState = newState => {
  switch (newState) {
  | Some(state) =>
    let json = JsonCoders.encodeState(state) |> Json.stringify;
    LocalStorage.setItem("notex-state", json);
  | None => LocalStorage.removeItem("notex-state")
  };

  state := newState;
  Future.value();
};

let saveStateAndNotify = newState =>
  saveState(newState)
  ->Future.tap(() => Belt.List.forEach(listeners^, l => l()));

let clear = () => saveStateAndNotify(None) |> ignore;

let subscribe = listener => listeners := [listener, ...listeners^];

let unsubscribe = listener =>
  listeners := Belt.List.keep(listeners^, l => l !== listener);

let getNotes_ = (state, notebookId) =>
  Belt.List.keep(state.notes, note => note.notebookId == notebookId);

let getNotes = notebookId =>
  Future.map(getState(), state => getNotes_(state, notebookId));

let getNotebooks = () =>
  Future.map(getState(), state =>
    state.notebooks
    |> List.map((n: Data.notebook) =>
         (n, getNotes_(state, n.id)->List.length)
       )
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

let addNotebooks = notebooks =>
  Future.map(
    getState(),
    state => {
      let newState = {
        ...state,
        notebooks: state.notebooks->(Belt.List.concat(notebooks)),
      };
      saveStateAndNotify(Some(newState)) |> ignore;
    },
  );

let addNotes = notes =>
  Future.map(
    getState(),
    state => {
      let newState = {
        ...state,
        notes: state.notes->(Belt.List.concat(notes)),
      };
      saveStateAndNotify(Some(newState)) |> ignore;
    },
  );

let addContentBlocks = contentBlocks =>
  Future.map(
    getState(),
    state => {
      let newState = {
        ...state,
        contentBlocks: state.contentBlocks->(Belt.List.concat(contentBlocks)),
      };
      saveStateAndNotify(Some(newState)) |> ignore;
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

      let newState = {...state, contentBlocks: updatedContentBlocks};
      Some(newState);
    })
  ->Future.tap(_ => sync ? DataSync.pushContentBlock(contentBlock) : ())
  ->Future.flatMap(saveStateAndNotify);

let insertRevision = (revision: string) =>
  Future.map(
    getState(),
    state => {
      let newState = {...state, revision: Some(revision)};
      saveStateAndNotify(Some(newState)) |> ignore;
    },
  );

let getRevision = () => Future.map(getState(), state => state.revision);
