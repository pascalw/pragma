let getToken = () => LocalStorage.getItem("pragma-token");

let check = () =>
  if (Belt.Option.isNone(getToken())) {
    let token = WindowRe.prompt("Enter token", Webapi.Dom.window);
    LocalStorage.setItem("pragma-token", token);
  };

let checkToken = (checkFn, token) =>
  checkFn(token)
  ->Future.tapOk(_ => LocalStorage.setItem("pragma-token", token));
