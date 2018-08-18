type tag = string;
type language = string;
type revision = Js.Date.t;

type content =
  | TextContent(string)
  | CodeContent(string, language);

type contentBlock = {
  id: int,
  noteId: int,
  content,
};

type note = {
  id: int,
  notebookId: int,
  title: string,
  tags: list(tag),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
};

type notebook = {
  id: int,
  name: string,
  createdAt: Js.Date.t,
};

type state = {
  revision: option(revision),
  notebooks: list(notebook),
  notes: list(note),
  contentBlocks: list(contentBlock),
};

type listener = unit => unit;

module JsonCoders = {
  /* Notebooks */
  let decodeNotebook = json =>
    Json.Decode.{
      id: json |> field("id", int),
      name: json |> field("name", string),
      createdAt: json |> field("createdAt", date),
    };

  let encodeNotebook = notebook =>
    Json.Encode.(
      object_([
        ("id", int(notebook.id)),
        ("name", string(notebook.name)),
        ("createdAt", date(notebook.createdAt)),
      ])
    );

  /* Notes */
  let decodeNote = json =>
    Json.Decode.{
      id: json |> field("id", int),
      notebookId: json |> field("notebookId", int),
      title: json |> field("title", string),
      tags: json |> field("tags", list(string)),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
    };

  let encodeNote = (note: note) =>
    Json.Encode.(
      object_([
        ("id", int(note.id)),
        ("notebookId", int(note.notebookId)),
        ("title", string(note.title)),
        ("tags", jsonArray(note.tags |> List.map(string) |> Array.of_list)),
        ("createdAt", date(note.createdAt)),
        ("updatedAt", date(note.updatedAt)),
      ])
    );

  /* ContentBlocks */
  let decodeContentBlock = json => {
    let textContent = json => {
      let text = json |> Json.Decode.field("text", Json.Decode.string);
      TextContent(text);
    };

    let codeContent = json => {
      let code = json |> Json.Decode.field("code", Json.Decode.string);
      let language =
        json |> Json.Decode.field("language", Json.Decode.string);

      CodeContent(code, language);
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
      id: json |> field("id", int),
      noteId: json |> field("noteId", int),
      content: json |> field("content", content),
    };
  };

  let encodeContentBlock = (contentBlock: contentBlock) => {
    let type_ = content =>
      switch (content) {
      | TextContent(_text) => "text"
      | CodeContent(_code, _language) => "code"
      };

    let data = content =>
      switch (content) {
      | TextContent(text) =>
        Json.Encode.(object_([("text", string(text))]))
      | CodeContent(code, language) =>
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
        ("id", int(contentBlock.id)),
        ("noteId", int(contentBlock.noteId)),
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
          | Some(revision) => date(revision)
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
      revision: json |> optional(field("revision", date)),
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

let saveStateAndNotify = newState => {
  switch (newState) {
  | Some(state) =>
    let json = JsonCoders.encodeState(state) |> Json.stringify;
    LocalStorage.setItem("notex-state", json);
  | None => LocalStorage.removeItem("notex-state")
  };

  state := newState;
  listeners^ |. Belt.List.forEach(l => l());
};

let clear = () => saveStateAndNotify(None);

let subscribe = listener => listeners := [listener, ...listeners^];

let getNotes_ = (state, notebookId) =>
  state.notes |. Belt.List.keep(note => note.notebookId == notebookId);

let getNotes = notebookId =>
  getState() |. Future.map(state => getNotes_(state, notebookId));

let getNotebooks = () =>
  getState()
  |. Future.map(state =>
       state.notebooks
       |> List.map(n => (n, getNotes_(state, n.id) |. List.length))
     );

let getContentBlocks = noteId =>
  getState()
  |. Future.map(state =>
       state.contentBlocks |. Belt.List.keep(cb => cb.noteId == noteId)
     );

let addNotebooks = notebooks =>
  getState()
  |. Future.map(state => {
       let newState = {
         ...state,
         notebooks: state.notebooks |. Belt.List.concat(notebooks),
       };
       saveStateAndNotify(Some(newState));
     });

let addNotes = notes =>
  getState()
  |. Future.map(state => {
       let newState = {
         ...state,
         notes: state.notes |. Belt.List.concat(notes),
       };
       saveStateAndNotify(Some(newState));
     });

let addContentBlocks = contentBlocks =>
  getState()
  |. Future.map(state => {
       let newState = {
         ...state,
         contentBlocks:
           state.contentBlocks |. Belt.List.concat(contentBlocks),
       };
       saveStateAndNotify(Some(newState));
     });

let insertRevision = (revision: Js.Date.t) =>
  getState()
  |. Future.map(state => {
       let newState = {...state, revision: Some(revision)};
       saveStateAndNotify(Some(newState));
     });

let getRevision = () => getState() |. Future.map(state => state.revision);
