open Belt;
open Data;

[@bs.module] external styles: Js.Dict.t(string) = "./NoteEditor.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

let component = ReasonReact.statelessComponent("NoteEditor");

let renderContentBlocks = (selectedNote: selectedNote, onChange) =>
  switch (selectedNote.content |> List.head) {
  | Some({id: _, content: TextContent(text) as content}) =>
    <TrixEditor
      key={selectedNote.note.id |> string_of_int}
      text
      onChange=(value => onChange(selectedNote.note, content, value))
    />
  | _ => <p> {ReasonReact.string("FIXME: unsupported content type.")} </p>
  };

let make = (~note, ~onChange, _children) => {
  ...component,
  render: _self =>
    switch (note) {
    | None =>
      <div className={style("editor")}>
        <p> {ReasonReact.string("No note selected")} </p>
      </div>
    | Some(selectedNote) =>
      <div className={style("editor")}>
        <h2 className={style("note-title")}>
          selectedNote.note.title->ReasonReact.string
        </h2>
        <div className="content">
          {renderContentBlocks(selectedNote, onChange)}
        </div>
      </div>
    },
};
