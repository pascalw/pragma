module JsonCoders = {
  let decodePendingChanges = Json.Decode.(list(string));

  let encodePendingChanges = changeIds =>
    changeIds->Array.of_list->Json.Encode.stringArray;
};

let store = changeIds => {
  let json = JsonCoders.encodePendingChanges(changeIds) |> Json.stringify;
  LocalStorage.setItem("notex-sync-state", json);
};

let getStoredChangeIds = () =>
  switch (LocalStorage.getItem("notex-sync-state")) {
  | None => []
  | Some(json) =>
    let json = Json.parseOrRaise(json);
    JsonCoders.decodePendingChanges(json);
  };
