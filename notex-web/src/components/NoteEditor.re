open Belt;
open Data;

[@bs.module] external styles: Js.Dict.t(string) = "./NoteEditor.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

let component = ReasonReact.statelessComponent("NoteEditor");

let renderContentBlocks = (note: Data.note, contentBlocks, onChange) =>
  switch (contentBlocks |> List.head) {
  | Some({id: _, content: TextContent(text)} as contentBlock) =>
    <TrixEditor
      key={note.id}
      text
      onChange=(value => onChange(contentBlock, value))
    />
  | _ => <p> {ReasonReact.string("FIXME: unsupported content type.")} </p>
  };

let make = (~note, ~contentBlocks, ~onChange, _children) => {
  ...component,
  render: _self =>
    switch (note) {
    | None => <div className={style("editor")} />
    | Some(note) =>
      <div className={style("editor")}>
        <h2 className={style("note-title")}>
          note.title->ReasonReact.string
        </h2>
        <div className="content">
          {
            renderContentBlocks(note, contentBlocks |> Option.getExn, onChange)
          }
        </div>
      </div>
    },
};
