[@bs.val] external hot: bool = "module.hot";

[@bs.val] external accept: unit => unit = "module.hot.accept";

let log = something => {
  Js.log(something);
  something;
};

let compareDates = (a, b) => {
  let aTime = Js.Date.getTime(a);
  let bTime = Js.Date.getTime(b);

  if (aTime > bTime) {
    1;
  } else if (aTime < bTime) {
    (-1);
  } else {
    0;
  };
};

let nanoIdAlphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
[@bs.module]
external generateNanoId: (string, int) => string = "nanoid/generate";
let generateId = () => generateNanoId(nanoIdAlphabet, 10);

let find = (xs: list('a), predicate: 'a => bool): option('a) =>
  Belt.List.keep(xs, predicate)->Belt.List.head;

let stripHtml = string => {
  open Webapi.Dom;

  let element = Document.createElement("div", document);
  ElementRe.setInnerHTML(element, string);
  ElementRe.innerText(element);
};
