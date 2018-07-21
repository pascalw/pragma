type tag = string;
type language = string;

type content =
  | TextContent(string)
  | CodeContent(string, language);

type note = {
  id: int,
  title: string,
  tags: list(tag),
  contentBlocks: list(content),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
};

type notebook = {
  id: int,
  name: string,
  notes: list(note),
  createdAt: Js.Date.t,
};
