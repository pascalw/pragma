exception UnknownContentType(string);

type resource = {
  id: int,
  type_: string,
};

type changes = {
  notebooks: list(Data.notebook),
  notes: list(Data.note),
  contentBlocks: list(Data.contentBlock),
};

type apiResponse = {
  revision: string,
  changes,
  deletions: list(resource),
};

module JsonCoders = {
  let encodeNotebook = (notebook: Data.notebook) =>
    Json.Encode.(
      object_([
        ("id", string(notebook.id)),
        ("name", string(notebook.name)),
        ("createdAt", date(notebook.createdAt)),
      ])
    );

  let decodeNotebook = json: Data.notebook =>
    Json.Decode.{
      id: json |> field("id", string),
      name: json |> field("name", string),
      createdAt: json |> field("createdAt", date),
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
      ])
    );

  let decodeNote = json: Data.note =>
    Json.Decode.{
      id: json |> field("id", string),
      notebookId: json |> field("notebookId", string),
      title: json |> field("title", string),
      tags: json |> field("tags", list(string)),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
    };

  let textContent = json => {
    let text = json |> Json.Decode.field("text", Json.Decode.string);
    Data.TextContent(text);
  };

  let codeContent = json => {
    let code = json |> Json.Decode.field("code", Json.Decode.string);
    let language = json |> Json.Decode.field("language", Json.Decode.string);

    Data.CodeContent(code, language);
  };

  let decodeContent = json => {
    let type_ = json |> Json.Decode.field("type", Json.Decode.string);

    switch (type_) {
    | "text" => json |> Json.Decode.field("data", textContent)
    | "code" => json |> Json.Decode.field("data", codeContent)
    | other => UnknownContentType(other)->raise
    };
  };

  let decodeContentBlock = json: Data.contentBlock =>
    Json.Decode.{
      id: json |> field("id", string),
      noteId: json |> field("noteId", string),
      content: json |> field("content", decodeContent),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
    };

  let encodeContentBlock = (contentBlock: Data.contentBlock) => {
    let type_ = (content: Data.content) =>
      switch (content) {
      | Data.TextContent(_text) => "text"
      | Data.CodeContent(_code, _language) => "code"
      };

    let data = (content: Data.content) =>
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

    let content = (content: Data.content) =>
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
      ])
    );
  };

  let decodeChanges = json =>
    Json.Decode.{
      notebooks: json |> field("notebooks", list(decodeNotebook)),
      notes: json |> field("notes", list(decodeNote)),
      contentBlocks:
        json |> field("contentBlocks", list(decodeContentBlock)),
    };

  let decodeResource = json =>
    Json.Decode.{
      id: json |> field("id", int),
      type_: json |> field("type", string),
    };

  let decodeChangesResponse = json =>
    Json.Decode.{
      revision: json |> field("revision", string),
      changes: json |> field("changes", decodeChanges),
      deletions: json |> field("deletions", list(decodeResource)),
    };
};

let fetchUrl = (revision: option(string)) =>
  switch (revision) {
  | Some(revision) => "/api/data?since_revision=" ++ revision
  | None => "/api/data"
  };

let toFuture = promise => {
  let promise =
    promise
    |> Js.Promise.then_(response =>
         if (!Fetch.Response.ok(response)) {
           Js.Promise.reject(
             Js.Exn.raiseError(
               "Request failed with " ++ Fetch.Response.statusText(response),
             ),
           );
         } else {
           Js.Promise.resolve(response);
         }
       );

  FutureJs.fromPromise(promise, Js.String.make);
};

let fetchChanges = (revision: option(string)) =>
  (
    fetchUrl(revision) |> Fetch.fetch |> Js.Promise.then_(Fetch.Response.json)
  )
  ->(FutureJs.fromPromise(Js.String.make)) /* FIXME */
  ->(Future.mapOk(JsonCoders.decodeChangesResponse));

let updateContentBlock = (contentBlock: Data.contentBlock) => {
  let json = JsonCoders.encodeContentBlock(contentBlock);

  Fetch.fetchWithInit(
    "/api/content_blocks/" ++ contentBlock.id,
    Fetch.RequestInit.make(
      ~method_=Put,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};

let createNote = (note: Data.note) => {
  let json = JsonCoders.encodeNote(note);

  Fetch.fetchWithInit(
    "/api/notes",
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};

let createNotebook = (notebook: Data.notebook) => {
  let json = JsonCoders.encodeNotebook(notebook);

  Fetch.fetchWithInit(
    "/api/notebooks",
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};

let updateNotebook = (notebook: Data.notebook) => {
  let json = JsonCoders.encodeNotebook(notebook);

  Fetch.fetchWithInit(
    "/api/notesbooks/" ++ notebook.id,
    Fetch.RequestInit.make(
      ~method_=Put,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};

let updateNote = (note: Data.note) => {
  let json = JsonCoders.encodeNote(note);

  Fetch.fetchWithInit(
    "/api/notes/" ++ note.id,
    Fetch.RequestInit.make(
      ~method_=Put,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};

let createContentBlock = (contentBlock: Data.contentBlock) => {
  let json = JsonCoders.encodeContentBlock(contentBlock);

  Fetch.fetchWithInit(
    "/api/content_blocks",
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(json)),
      ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
      (),
    ),
  )
  ->toFuture;
};
