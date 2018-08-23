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
};
type notebook = {
  id: int,
  name: string,
  createdAt: Js.Date.t,
};

type listener = unit => unit;
let subscribe: listener => unit;
let addNotebooks: list(notebook) => Future.t(unit);
let addNotes: list(note) => Future.t(unit);

let getNotes: int => Future.t(list(note));
let getNotebooks: unit => Future.t(list((notebook, int)));

let getContentBlocks: int => Future.t(list(contentBlock));
let addContentBlocks: list(contentBlock) => Future.t(unit);
let updateContentBlock: contentBlock => Future.t(unit);

let clear: unit => unit;

let insertRevision: Js.Date.t => Future.t(unit);
let getRevision: unit => Future.t(option(Js.Date.t));
