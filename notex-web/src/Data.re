type tag = string;
type language = string;

type content =
  | TextContent(string)
  | CodeContent(string, language);

type contentBlock = {
  id: string,
  noteId: string,
  content,
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};

type note = {
  id: string,
  notebookId: string,
  title: string,
  tags: list(tag),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};

type notebook = {
  id: string,
  name: string,
  createdAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};
