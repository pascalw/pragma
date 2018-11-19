type tag = string;
type language = string;

type content =
  | TextContent(RichText.t)
  | CodeContent(string, language);

type contentBlock = {
  id: string,
  noteId: string,
  content,
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  revision: option(string),
};

type note = {
  id: string,
  notebookId: string,
  title: string,
  tags: list(tag),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  revision: option(string),
};

type notebook = {
  id: string,
  title: string,
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  revision: option(string),
};

let newNotebook = () : notebook => {
  let now = Js.Date.fromFloat(Js.Date.now());

  {
    id: Utils.generateId(),
    title: "Untitled notebook",
    createdAt: now,
    updatedAt: now,
    revision: None,
  };
};

let newContentBlock = (noteId) : contentBlock => {
  let now = Js.Date.fromFloat(Js.Date.now());

  {
    id: Utils.generateId(),
    noteId,
    content: TextContent(RichText.create()),
    createdAt: now,
    updatedAt: now,
    revision: None
  }
};
