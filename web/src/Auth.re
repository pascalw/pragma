let getToken = () => LocalStorage.getItem("pragma-token");

let checkToken = (checkFn, token) =>
  checkFn(token)
  ->Future.tapOk(_ => LocalStorage.setItem("pragma-token", token));
