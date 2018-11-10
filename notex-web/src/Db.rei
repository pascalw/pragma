type listener = unit => unit;

let subscribe: listener => unit;
let unsubscribe: listener => unit;

let addNotebook: Data.notebook => Future.t(unit);
let addNote: Data.note => Future.t(unit);

let createNote: string => Future.t((Data.note, Data.contentBlock));
let createNotebook: Data.notebook => Future.t(Data.notebook);

let getNote: string => Future.t(option(Data.note));
let getNotes: string => Future.t(list(Data.note));
let getNotebooks: unit => Future.t(list((Data.notebook, int)));
let getNotebook: string => Future.t(option(Data.notebook));

let getContentBlocks: string => Future.t(list(Data.contentBlock));
let getContentBlock: string => Future.t(option(Data.contentBlock));
let addContentBlock: Data.contentBlock => Future.t(unit);

let updateContentBlock: (Data.contentBlock, ~sync: bool=?, unit) => Future.t(unit);
let updateNote: (Data.note, ~sync:bool=?, unit) => Future.t(unit);
let updateNotebook: (Data.notebook, ~sync:bool=?, unit) => Future.t(unit);

let deleteNotebook: (string, ~sync: bool=?, unit) => Future.t(unit);
let deleteNote: (string, ~sync: bool=?, unit) => Future.t(unit);
let deleteContentBlock: (string) => Future.t(unit);

let withNotification: (unit => 'a) => 'a;

let clear: unit => unit;

let insertRevision: string => Future.t(unit);
let getRevision: unit => Future.t(option(string));
