type tag = string;
type language = string;

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
  systemUpdatedAt: Js.Date.t,
};

type selectedNote = {
  note,
  content: list(contentBlock),
};

type notebook = {
  id: int,
  name: string,
  createdAt: Js.Date.t,
  systemUpdatedAt: Js.Date.t,
};

type selectedNotebook = {
  notebook,
  notes: list(note),
};
