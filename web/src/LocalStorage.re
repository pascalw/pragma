[@bs.scope ("window", "localStorage")] [@bs.val]
external getItemRaw: string => Js.Nullable.t(string) = "getItem";

let rawKeys: unit => array(string) = [%bs.raw
  {|
  function keys() {
    return Object.keys(localStorage);
  }
|}
];

let keys = () => rawKeys() |> Belt.List.fromArray;

let getItem = key => getItemRaw(key) |> Js.Nullable.toOption;

[@bs.scope ("window", "localStorage")] [@bs.val] external setItem: (string, string) => unit = "";

[@bs.scope ("window", "localStorage")] [@bs.val] external clear: unit => unit = "";

[@bs.scope ("window", "localStorage")] [@bs.val] external removeItem: string => unit = "";
