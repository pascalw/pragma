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

let renderContentBlock =
    (note: Data.note, onChange, contentBlock: Data.contentBlock) =>
  <div className={style("contentBlock")} key={contentBlock.id}>
    {
      switch (contentBlock) {
      | {content: TextContent(text)} =>
        <TrixEditor
          key={note.id}
          text
          autoFocus={!isUntitled(note)}
          onChange=(value => onChange(Text(contentBlock, value)))
        />
      | {content: CodeContent(_, _)} =>
        <CodeEditor
          key={note.id}
          contentBlock
          onChange=(value => onChange(Text(contentBlock, value)))
        />
      }
    }
  </div>;

let renderContentBlocks = (note: Data.note, contentBlocks, onChange) =>
  Belt.List.map(contentBlocks, renderContentBlock(note, onChange));

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
        {
          renderContentBlocks(note, contentBlocks, onChange)
          |> Belt.List.toArray
          |> ReasonReact.array
        }
      </div>
    </div>,
};
