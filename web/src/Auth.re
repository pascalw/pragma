let getToken = () => LocalStorage.getItem("pragma-token");

let checkToken = (checkFn, token) =>
  checkFn(token)
  |> Promises.mapOk(result => {
       LocalStorage.setItem("pragma-token", token);
       result;
     });
