let clearStorage = () => Db.clear();

let _install: (unit => unit) => unit = [%bs.raw
  {|
  function installDevtools(clearStorage) {
    window.devtools = { clearStorage: clearStorage };
  }
|}
];

let install = () => _install(() => clearStorage() |> ignore);
