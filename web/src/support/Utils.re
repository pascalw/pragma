[@bs.val] external hot: bool = "module.hot";

[@bs.val] external accept: unit => unit = "module.hot.accept";

module Import = {
  [@bs.val] external import: string => Js.Promise.t('a) = "import";
};

let dictSet = (dict, key, value) => {
  Js.Dict.set(dict, key, value);
  dict;
};

let log = (something, ~label="", ()) => {
  Js.log2(label, something);
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

let timeElapsedSince = (date: Js.Date.t): float => {
  Js.Date.now() -. Js.Date.getTime(date);
};

let now = () => Js.Date.fromFloat(Js.Date.now());

let nanoIdAlphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
[@bs.module] external generateNanoId: (string, int) => string = "nanoid/generate";
let generateId = () => generateNanoId(nanoIdAlphabet, 10);

let find = (xs: list('a), predicate: 'a => bool): option('a) =>
  Belt.List.keep(xs, predicate)->Belt.List.head;

let textToHtml = text => Js.String.replaceByRe([%re "/(?:\\r\\n|\\r|\\n)/g"], "<br/>", text);

let htmlToText: string => string = [%bs.raw
  {|
// https://github.com/eldios/htmlToText/blob/master/jsHtmlToText.js
function htmlToText(html) {
  return html
    // Remove line breaks
    .replace(/(?:\n|\r\n|\r)/ig,"")
    // Turn <br>'s into single line breaks.
    .replace(/<\s*br[^>]*>/ig,"\n")
    // Turn </li>'s into line breaks.
     .replace(/<\s*\/li[^>]*>/ig,"\n")
    // Turn <p>'s into double line breaks.
     .replace(/<\s*p[^>]*>/ig,"\n\n")
    // Remove content in script tags.
     .replace(/<\s*script[^>]*>[\s\S]*?<\/script>/mig,"")
    // Remove content in style tags.
     .replace(/<\s*style[^>]*>[\s\S]*?<\/style>/mig,"")
    // Remove content in comments.
     .replace(/<!--.*?-->/mig,"")
     // Format anchor tags properly.
     // e.g.
     // input - <a class='ahref' href='http://pinetechlabs.com/' title='asdfqwer\"><b>asdf</b></a>
     // output - asdf (http://pinetechlabs.com/)
     .replace(/<\s*a[^>]*href=['"](.*?)['"][^>]*>([\s\S]*?)<\/\s*a\s*>/ig, "$2 ($1)")
    // Remove all remaining tags.
     .replace(/(<([^>]+)>)/ig,"")
    // Make sure there are never more than two
    // consecutive linebreaks.
     .replace(/\n{2,}/g,"\n\n")
    // Remove tabs.
     .replace(/\t/g,"")
    // Remove newlines at the beginning of the text.
     .replace(/^\n+/m,"")
    // Replace multiple spaces with a single space.
    .replace(/ {2,}/g," ");
}
|}
];

/* Based on https://stackoverflow.com/a/53711623/6769663 */
let isUrl =
  Js.Re.test(
    _,
    [%bs.re
      "/^((?:https?):\\/\\/?)?([^:/\\s.]+\\.[^:/\\s]|localhost)(:\\d+)?((?:\\/\\w+)*\\/)?([\\w\\-.]+[^#?\\s]+)?([^#]+)?(#[\\w-]+)?$/i"
    ],
  );

[@bs.module "classnames"] [@bs.splice] external classnames: array(string) => string = "default";

module DayJs = {
  type t;
  [@bs.module "dayjs"] external make: Js.Date.t => t = "default";
  [@bs.send] external format: (t, string) => string = "format";
};

let formatDate = (date, format) => date->DayJs.make->DayJs.format(format);

let benchmark = (label: string, func: 'a => 'b): 'b => {
  let start = Js.Date.now();
  let result = func();
  Js.log3(label, "took ", Js.Date.now() -. start);
  result;
};

let benchmarkCb = (label: string): (unit => unit) => {
  let start = Js.Date.now();
  () => {
    Js.log3(label, "took ", Js.Date.now() -. start);
  };
};

type removeListener = unit => unit;
let buildVisibilityChangeListener =
    (desiredState: DomTypesRe.visibilityState, document, listener: unit => unit): removeListener => {
  open Webapi;

  let domListener = _e =>
    if (Dom.Document.visibilityState(document) == desiredState) {
      listener();
    };

  Dom.Document.addEventListener("visibilitychange", domListener, document);

  () => {
    Dom.Document.removeEventListener("visibilitychange", domListener, document);
  };
};

let onPageVisible = buildVisibilityChangeListener(DomTypesRe.Visible, Webapi.Dom.document);
