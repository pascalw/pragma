open Belt;
open Data;

[@bs.module] external styles: Js.Dict.t(string) = "./NoteEditor.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

let component = ReasonReact.statelessComponent("NoteEditor");

let isUntitled = (note: Data.note) => note.title == "Untitled note";
let title = note => isUntitled(note) ? "" : note.title;

type change =
  | Content(Data.contentBlock, Data.content)
  | ContentBlock(Data.contentBlock)
  | Title(Data.note, string);

let onChangeTitle = (note, onChange, e) => {
  let value = ReactEvent.Form.target(e)##value;
  onChange(Title(note, value));
};

let blockClass = contentBlock =>
  switch (contentBlock.content) {
  | TextContent(_) => "text"
  | CodeContent(_, _) => "code"
  };

let blockStringType = contentBlock =>
  switch (contentBlock.content) {
  | TextContent(_) => "text"
  | CodeContent(_, language) => language
  };

let onContentBlockTypeChange = (contentBlock, onChange, event) => {
  let value = ReactEvent.Form.target(event)##value;
  let updatedBlock =
    switch (value) {
    | "text" => ContentBlocks.updateContentType(contentBlock, value)
    | codeLanguage =>
      ContentBlocks.(
        contentBlock
        ->updateContentType("code")
        ->updateCodeLanguage(codeLanguage)
      )
    };

  onChange(ContentBlock(updatedBlock));
};

let renderContentBlock = (onChange, onShiftEnter, onDeleteIntent, contentBlock: Data.contentBlock) =>
  <div
    className={style("contentBlock") ++ " " ++ blockClass(contentBlock)}
    key={contentBlock.id}>
    <select
      tabIndex=(-1)
      className={style("typeSelector")}
      value={blockStringType(contentBlock)}
      onChange={onContentBlockTypeChange(contentBlock, onChange)}>
      <option value="text"> {ReasonReact.string("Text")} </option>
      {CodeEditor.typeOptions()}
    </select>
    {
      switch (contentBlock) {
      | {content: TextContent(richText)} =>
        <RichTextEditor
          key={contentBlock.id}
          onChange=(
            value => onChange(Content(contentBlock, TextContent(value)))
          )
          onShiftEnter
          onDeleteIntent={() => onDeleteIntent(contentBlock)}
          value=richText
        />
      | {content: CodeContent(_, language)} =>
        <CodeEditor
          key={contentBlock.id}
          contentBlock
          onChange=(
            value =>
              onChange(Content(contentBlock, CodeContent(value, language)))
          )
        />
      }
    }
  </div>;

let sortAsc = (input: list('a), mapper: 'a => Js.Date.t) =>
Belt.List.sort(input, (a, b) =>
  Utils.compareDates(mapper(a), mapper(b))
);

let renderContentBlocks = (contentBlocks: list(Data.contentBlock), onChange, onShiftEnter, onDeleteIntent) => {
  contentBlocks
  ->sortAsc(block => block.createdAt)
  ->Belt.List.map(renderContentBlock(onChange, onShiftEnter, onDeleteIntent));
};

let make = (~note: Data.note, ~contentBlocks, ~onChange, ~onShiftEnter, ~onDeleteIntent, _children) => {
  ...component,
  render: _self =>
    <div className={style("editor") ++ " " ++ style("content")}>
      <input
        className={style("note-title")}
        placeholder="Untitled note"
        key={note.id}
        autoFocus={isUntitled(note)}
        value={title(note)}
        onChange={onChangeTitle(note, onChange)}
      />
      {
        renderContentBlocks(contentBlocks, onChange, onShiftEnter, onDeleteIntent)
        |> Belt.List.toArray
        |> ReasonReact.array
      }
    </div>,
};
