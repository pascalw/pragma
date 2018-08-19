exception UnknownContentType(string);

type resource = {
  id: int,
  type_: string,
};

type notebook = {
  id: int,
  name: string,
  createdAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};

type note = {
  id: int,
  notebookId: int,
  title: string,
  tags: list(string),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};

type content =
  | TextContent(string)
  | CodeContent(string, string);

type contentBlock = {
  id: int,
  noteId: int,
  content,
};

type changes = {
  notebooks: list(notebook),
  notes: list(note),
  contentBlocks: list(contentBlock),
};

type apiResponse = {
  revision: Js.Date.t,
  changes,
  deletions: list(resource),
};

module Decode = {
  let notebook = json =>
    Json.Decode.{
      id: json |> field("id", int),
      name: json |> field("name", string),
      createdAt: json |> field("createdAt", date),
      systemUpdatedAt: json |> field("systemUpdatedAt", date),
    };

  let note = json =>
    Json.Decode.{
      id: json |> field("id", int),
      notebookId: json |> field("notebookId", int),
      title: json |> field("title", string),
      tags: json |> field("tags", list(string)),
      createdAt: json |> field("createdAt", date),
      updatedAt: json |> field("updatedAt", date),
      systemUpdatedAt: json |> field("systemUpdatedAt", date),
    };

  let textContent = json => {
    let text = json |> Json.Decode.field("text", Json.Decode.string);
    TextContent(text);
  };

  let codeContent = json => {
    let code = json |> Json.Decode.field("code", Json.Decode.string);
    let language = json |> Json.Decode.field("language", Json.Decode.string);

    CodeContent(code, language);
  };

  let content = json => {
    let type_ = json |> Json.Decode.field("type", Json.Decode.string);

    switch (type_) {
    | "text" => json |> Json.Decode.field("data", textContent)
    | "code" => json |> Json.Decode.field("data", codeContent)
    | other => UnknownContentType(other)->raise
    };
  };

  let contentBlock = json =>
    Json.Decode.{
      id: json |> field("id", int),
      noteId: json |> field("noteId", int),
      content: json |> field("content", content),
    };

  let changes = json =>
    Json.Decode.{
      notebooks: json |> field("notebooks", list(notebook)),
      notes: json |> field("notes", list(note)),
      contentBlocks: json |> field("contentBlocks", list(contentBlock)),
    };

  let resource = json =>
    Json.Decode.{
      id: json |> field("id", int),
      type_: json |> field("type", string),
    };

  let apiResponse = json =>
    Json.Decode.{
      revision: json |> field("revision", date),
      changes: json |> field("changes", changes),
      deletions: json |> field("deletions", list(resource)),
    };
};

let fetchUrl = (revision: option(Js.Date.t)) =>
  switch (revision) {
  | Some(revision) =>
    "/api/data?since_revision=" ++ Js.Date.toISOString(revision)
  | None => "/api/data"
  };

let fetchChanges = (revision: option(Js.Date.t)) =>
  (
    fetchUrl(revision) |> Fetch.fetch |> Js.Promise.then_(Fetch.Response.json)
  )
  ->(FutureJs.fromPromise(Js.String.make)) /* FIXME */
  ->(Future.mapOk(Decode.apiResponse));
