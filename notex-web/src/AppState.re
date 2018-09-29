type state = {
  selectedNotebookId: option(string),
  selectedNoteId: option(string),
};

module JsonCoders = {
  let decodeState = json: state =>
    Json.Decode.{
      selectedNotebookId:
        json |> optional(field("selectedNotebookId", string)),
      selectedNoteId: json |> optional(field("selectedNoteId", string)),
    };

  let encodeState = (state: state) =>
    Json.Encode.(
      object_([
        (
          "selectedNoteId",
          switch (state.selectedNoteId) {
          | None => null
          | Some(id) => string(id)
          },
        ),
        (
          "selectedNotebookId",
          switch (state.selectedNotebookId) {
          | None => null
          | Some(id) => string(id)
          },
        ),
      ])
    );
};

let get = () =>
  LocalStorage.getItem("notex-app-state")
  ->Belt.Option.map(Json.parseOrRaise)
  ->Belt.Option.map(json => JsonCoders.decodeState(json))
  ->Belt.Option.getWithDefault({
      selectedNotebookId: None,
      selectedNoteId: None,
    });

let saveState = state => {
  let json = JsonCoders.encodeState(state) |> Json.stringify;
  LocalStorage.setItem("notex-app-state", json);
};

let setSelected = (notebookId, noteId) =>
  saveState({selectedNotebookId: notebookId, selectedNoteId: noteId});
