type listener = unit => unit;
let subscribe: listener => unit;
let addNotebooks: list(Data.notebook) => Future.t(unit);
let addNotes: list(Data.note) => Future.t(unit);

let getNotes: int => Future.t(list(Data.note));
let getNotebooks: unit => Future.t(list((Data.notebook, int)));

let getContentBlocks: int => Future.t(list(Data.contentBlock));
let getContentBlock: int => Future.t(option(Data.contentBlock));
let addContentBlocks: list(Data.contentBlock) => Future.t(unit);
let updateContentBlock: (Data.contentBlock, ~sync: bool=?, unit) => Future.t(unit);

let clear: unit => unit;

let insertRevision: string => Future.t(unit);
let getRevision: unit => Future.t(option(string));
