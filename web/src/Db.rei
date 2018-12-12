type result('a) = Belt.Result.t('a, Js.Promise.error);
type listener = unit => unit;

let subscribe: listener => unit;
let unsubscribe: listener => unit;

let addNotebook: Data.notebook => Repromise.t(result(unit));
let addNote: Data.note => Repromise.t(result(unit));

let createNote: string => Repromise.t(result((Data.note, Data.contentBlock)));
let createNotebook: Data.notebook => Repromise.t(result(Data.notebook));

let getNote: string => Repromise.t(option(Data.note));
let getNotes: string => Repromise.t(list(Data.note));
let getNotebooks: unit => Repromise.t(list((Data.notebook, int)));
let getNotebook: string => Repromise.t(option(Data.notebook));

let getContentBlocks: string => Repromise.t(list(Data.contentBlock));
let getContentBlock: string => Repromise.t(option(Data.contentBlock));
let addContentBlock: Data.contentBlock => Repromise.t(result(unit));

let updateContentBlock: (Data.contentBlock, ~sync: bool=?, unit) => Repromise.t(result(unit));
let updateNote: (Data.note, ~sync:bool=?, unit) => Repromise.t(result(unit));
let updateNotebook: (Data.notebook, ~sync:bool=?, unit) => Repromise.t(result(unit));

let deleteNotebook: (string, ~sync: bool=?, unit) => Repromise.t(result(unit));
let deleteNote: (string, ~sync: bool=?, unit) => Repromise.t(result(unit));
let deleteContentBlock: (string) => Repromise.t(result(unit));

let withNotification: (unit => 'a) => 'a;
let withPromiseNotification: Repromise.t('a) => unit; 

let insertRevision: string => Repromise.t(unit);
let getRevision: unit => Repromise.t(option(string));
let clear: unit => unit;
