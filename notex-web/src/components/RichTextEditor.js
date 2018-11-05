import React from "react";
import { Editor, RichUtils } from "draft-js";
import { handleReturnInList } from "../draft-js/list-behavior";
import classNames from "classnames/bind";
import styles from "./RichTextEditor.scss";
import { jsComponent as ButtonBar } from "./RichTextButtonBar.bs";

const styleMap = {
  STRIKETHROUGH: {
    textDecoration: "line-through"
  }
};

let cx = classNames.bind(styles);

export class RichTextEditor extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      value: props.value
    };

    this.focus = () => this.editor.focus();
    this.isFocused = () => {
      return this.state.value.getSelection().getHasFocus();
    };
  }

  onTab = e => {
    const newState = RichUtils.onTab(e, this.state.value, 4 /* maxDepth */);

    if (newState) {
      this.onChange(newState);
    }
  };

  onChange = state => {
    if (state.getCurrentContent() !== this.state.value.getCurrentContent()) {
      this.props.onChange(state);
    }

    this.setState({ value: state });
  };

  handleReturn = (_e, editorState) => {
    const newState = handleReturnInList(editorState);

    if (newState) {
      this.onChange(newState);
      return "handled";
    }

    return "not-handled";
  };

  handleKeyCommand = (command, editorState) => {
    if (command == "backspace") {
      // handle backspace like we handle return,
      // acts only on empty list items.
      return this.handleReturn(null, editorState);
    }

    const newState = RichUtils.handleKeyCommand(editorState, command);
    if (newState) {
      this.onChange(newState);
      return "handled";
    }

    return "not-handled";
  };

  toggleBlockType = blockType => {
    this.onChange(RichUtils.toggleBlockType(this.state.value, blockType));
  };

  toggleInlineStyle = inlineStyle => {
    this.onChange(RichUtils.toggleInlineStyle(this.state.value, inlineStyle));
  };

  currentBlockType = () => {
    const editorState = this.state.value;

    const selection = editorState.getSelection();

    return editorState
      .getCurrentContent()
      .getBlockForKey(selection.getStartKey())
      .getType();
  };

  currentStyle = () => {
    return this.state.value.getCurrentInlineStyle();
  };

  render() {
    const className = cx({
      editor: true,
      editorFocused: this.isFocused()
    });

    const currentStyle = this.currentStyle();
    const currentBlockType = this.currentBlockType();

    return (
      <div className={className} onClick={this.focus}>
        <Editor
          ref={ref => (this.editor = ref)}
          editorState={this.state.value}
          onChange={this.onChange}
          onTab={this.onTab}
          customStyleMap={styleMap}
          handleReturn={this.handleReturn}
          handleKeyCommand={this.handleKeyCommand}
        />

        <ButtonBar
          toggleInlineStyle={this.toggleInlineStyle}
          toggleBlockType={this.toggleBlockType}
          isStyleActive={currentStyle.has.bind(currentStyle)}
          currentBlockType={currentBlockType}
        />
      </div>
    );
  }
}
