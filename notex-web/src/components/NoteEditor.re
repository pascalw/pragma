open Belt;
open Data;

[@bs.module] external styles: Js.Dict.t(string) = "./NoteEditor.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

let component = ReasonReact.statelessComponent("NoteEditor");

let isUntitled = (note: Data.note) => note.title == "Untitled note";
let title = note => isUntitled(note) ? "" : note.title;

type change =
  | Text(Data.contentBlock, string)
  | Title(Data.note, string);

let onChangeTitle = (note, onChange, e) => {
  let value = ReactEvent.Form.target(e)##value;
  onChange(Title(note, value));
};

let renderContentBlocks = (note: Data.note, contentBlocks, onChange) =>
  switch (contentBlocks |> List.head) {
  | Some({content: TextContent(text)} as contentBlock) =>
    <TrixEditor
      key={note.id}
      text
      autoFocus={!isUntitled(note)}
      onChange=(value => onChange(Text(contentBlock, value)))
    />
  | Some({content: CodeContent(_, _)} as contentBlock) =>
    <CodeEditor
      key={note.id}
      contentBlock
      onChange=(value => onChange(Text(contentBlock, value)))
    />
  | _ => <p> {ReasonReact.string("FIXME: unsupported content type.")} </p>
  };

let make = (~note: Data.note, ~contentBlocks, ~onChange, _children) => {
  ...component,
  render: _self =>
    <div className={style("editor")}>
      <input
        className={style("note-title")}
        placeholder="Untitled note"
        key={note.id}
        autoFocus={isUntitled(note)}
        value={title(note)}
        onChange={onChangeTitle(note, onChange)}
      />
      <div className="content">
        {renderContentBlocks(note, contentBlocks, onChange)}
      </div>
    </div>,
};
