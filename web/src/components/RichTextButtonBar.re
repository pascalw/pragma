[@bs.module] external styles: Js.Dict.t(string) = "./RichTextButtonBar.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

module Button = {
  let component = ReasonReact.statelessComponent("Button");
  let make = (~onMouseDown, ~isActive, ~title, children) => {
    ...component,
    render: _self => {
      let className =
        isActive ?
          Utils.classnames([|style("button"), style("buttonActive")|]) :
          style("button");

      <button onMouseDown className title> ...children </button>;
    },
  };
};

module InlineStyleButton = {
  let component = ReasonReact.statelessComponent("InlineStyleButton");
  let make = (~toggleInlineStyle, ~isActive, ~styleName, ~title, children) => {
    ...component,
    render: _self => {
      let isActive = isActive(styleName);

      <Button isActive title onMouseDown={toggleInlineStyle(styleName)}>
        ...children
      </Button>;
    },
  };
};

module BlockTypeButton = {
  let component = ReasonReact.statelessComponent("BlockTypeButton");
  let make =
      (~toggleBlockType, ~currentBlockType, ~blockType, ~title, children) => {
    ...component,
    render: _self => {
      let isActive = blockType == currentBlockType;

      <Button title isActive onMouseDown={toggleBlockType(blockType)}>
        ...children
      </Button>;
    },
  };
};

let component = ReasonReact.statelessComponent("RichTextButtonBar");
let make =
    (
      ~toggleInlineStyle,
      ~toggleBlockType,
      ~isStyleActive,
      ~currentBlockType,
      ~spellcheck,
      ~toggleSpellcheck,
      _children,
    ) => {
  let toggleInlineStyle = (style, event) => {
    ReactEvent.Mouse.preventDefault(event);
    toggleInlineStyle(style);
  };

  let toggleBlockType = (blockType, event) => {
    ReactEvent.Mouse.preventDefault(event);
    toggleBlockType(blockType);
  };

  let toggleSpellcheck = _event => toggleSpellcheck();

  {
    ...component,
    render: _self =>
      <div className={style("wrapper")}>
        <div className={style("buttonBar")}>
          <div>
            <InlineStyleButton
              title="Bold"
              styleName="BOLD"
              toggleInlineStyle
              isActive=isStyleActive>
              <Icon icon=Icon.FormatBold />
            </InlineStyleButton>
            <InlineStyleButton
              title="Italic"
              styleName="ITALIC"
              toggleInlineStyle
              isActive=isStyleActive>
              <Icon icon=Icon.FormatItalic />
            </InlineStyleButton>
            <InlineStyleButton
              title="Underline"
              styleName="UNDERLINE"
              toggleInlineStyle
              isActive=isStyleActive>
              <Icon icon=Icon.FormatUnderline />
            </InlineStyleButton>
            <InlineStyleButton
              title="Strikethrough"
              styleName="STRIKETHROUGH"
              toggleInlineStyle
              isActive=isStyleActive>
              <Icon icon=Icon.FormatStrikethrough />
            </InlineStyleButton>
          </div>
          <div> <div className={style("divider")} /> </div>
          <div>
            <BlockTypeButton
              title="Quote"
              blockType="blockquote"
              toggleBlockType
              currentBlockType>
              <Icon icon=Icon.FormatQuote />
            </BlockTypeButton>
            <BlockTypeButton
              title="Unordered list"
              blockType="unordered-list-item"
              toggleBlockType
              currentBlockType>
              <Icon icon=Icon.BulletList />
            </BlockTypeButton>
            <BlockTypeButton
              title="Ordered list"
              blockType="ordered-list-item"
              toggleBlockType
              currentBlockType>
              <Icon icon=Icon.NumberedList />
            </BlockTypeButton>
          </div>
          <div> <div className={style("divider")} /> </div>
          <div>
            <Button
              title="Spellcheck"
              isActive=spellcheck
              onMouseDown=toggleSpellcheck>
              <Icon icon=Icon.Spellcheck />
            </Button>
          </div>
        </div>
      </div>,
  };
};

[@bs.deriving abstract]
type jsProps = {
  toggleInlineStyle: string => unit,
  toggleBlockType: string => unit,
  isStyleActive: string => bool,
  currentBlockType: string,
  spellcheck: bool,
  toggleSpellcheck: unit => unit,
};

let jsComponent =
  ReasonReact.wrapReasonForJs(~component, jsProps =>
    make(
      ~toggleInlineStyle=jsProps->toggleInlineStyleGet,
      ~toggleBlockType=jsProps->toggleBlockTypeGet,
      ~isStyleActive=jsProps->isStyleActiveGet,
      ~currentBlockType=jsProps->currentBlockTypeGet,
      ~spellcheck=jsProps->spellcheckGet,
      ~toggleSpellcheck=jsProps->toggleSpellcheckGet,
      [||],
    )
  );
