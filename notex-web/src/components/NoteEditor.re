open Belt;
open Data;

[@bs.module] external styles: Js.Dict.t(string) = "./NoteEditor.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

let component = ReasonReact.statelessComponent("NoteEditor");

let isUntitled = (note: Data.note) => note.title == "Untitled note";
let title = note => isUntitled(note) ? "" : note.title;

type change =
  | Text(Data.contentBlock, string)
  | ContentBlock(Data.contentBlock)
  | Title(Data.note, string);

let onChangeTitle = (note, onChange, e) => {
  let value = ReactEvent.Form.target(e)##value;
  onChange(Title(note, value));
};

let blockStringType = contentBlock =>
  switch (contentBlock.content) {
  | TextContent(_) => "text"
  | CodeContent(_, _) => "code"
  };

let onContentBlockTypeChange = (contentBlock, onChange, event) => {
  let value = ReactEvent.Form.target(event)##value;
  onChange(
    ContentBlock(ContentBlocks.updateContentType(contentBlock, value)),
  );
};

let renderContentBlock =
    (note: Data.note, onChange, contentBlock: Data.contentBlock) =>
  <div className={style("contentBlock")} key={contentBlock.id}>
    <select
      className={style("contentBlockType")}
      value={blockStringType(contentBlock)}
      onChange={onContentBlockTypeChange(contentBlock, onChange)}>
      <option value="text"> {ReasonReact.string("Text block")} </option>
      <option value="code"> {ReasonReact.string("Code block")} </option>
    </select>
    {
      switch (contentBlock) {
      | {content: TextContent(text)} =>
        <TrixEditor
          key={contentBlock.id}
          text
          autoFocus={!isUntitled(note)}
          onChange=(value => onChange(Text(contentBlock, value)))
        />
      | {content: CodeContent(_, _)} =>
        <CodeEditor
          key={contentBlock.id}
          contentBlock
          onChange=(value => onChange(Text(contentBlock, value)))
          onLanguageChange=(
            language =>
              onChange(
                ContentBlock(
                  ContentBlocks.updateCodeLanguage(contentBlock, language),
                ),
              )
          )
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
