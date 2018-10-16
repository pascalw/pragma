let getToken = () => LocalStorage.getItem("notex-token");

let check = () =>
  if (Belt.Option.isNone(getToken())) {
    let token = WindowRe.prompt("Enter token", Webapi.Dom.window);
    LocalStorage.setItem("notex-token", token);
  };
