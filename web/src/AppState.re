module Theme = {
  type t =
    | Light
    | Dark;

  let toString =
    fun
    | Light => "light"
    | Dark => "dark";

  let fromString =
    fun
    | "light" => Some(Light)
    | "dark" => Some(Dark)
    | _ => None;
};

type state = {
  selectedNotebookId: option(string),
  selectedNoteId: option(string),
};

module JsonCoders = {
  let decodeState = (json): state =>
    Json.Decode.{
      selectedNotebookId: json |> optional(field("selectedNotebookId", string)),
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
  LocalStorage.getItem("pragma-app-state")
  ->Belt.Option.map(Json.parseOrRaise)
  ->Belt.Option.map(json => JsonCoders.decodeState(json))
  ->Belt.Option.getWithDefault({selectedNotebookId: None, selectedNoteId: None});

let saveState = state => {
  let json = JsonCoders.encodeState(state) |> Json.stringify;
  LocalStorage.setItem("pragma-app-state", json);
};

let setSelected = (notebookId, noteId) =>
  saveState({selectedNotebookId: notebookId, selectedNoteId: noteId});

let getTheme = () => LocalStorage.getItem("pragma-theme")->Belt.Option.flatMap(Theme.fromString);
