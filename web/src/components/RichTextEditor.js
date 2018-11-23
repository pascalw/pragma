import React from "react";
import { Editor, RichUtils } from "draft-js";
import {
  registerCopySource,
  handleDraftEditorPastedText,
} from "draftjs-conductor";
import { handleReturnInList } from "../draft-js/list-behavior";
import { isSelectionAtStart } from "../draft-js/utils";
import classNames from "classnames/bind";
import styles from "./RichTextEditor.scss";
import { jsComponent as ButtonBar } from "./RichTextButtonBar.bs";

const styleMap = {
  STRIKETHROUGH: {
    textDecoration: "line-through"
  }
};

let cx = classNames.bind(styles);

const spellcheckEnabled = () => {
  return window.localStorage.getItem("pragma-spellcheck") == "true";
};

const setSpellcheck = spellcheckState => {
  window.localStorage.setItem("pragma-spellcheck", spellcheckState);
};

export class RichTextEditor extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      value: props.value,
      spellcheck: spellcheckEnabled()
    };

    this.focus = () => this.editor.focus();
    this.isFocused = () => {
      return this.state.value.getSelection().getHasFocus();
    };
  }

  componentDidMount() {
    this.copySource = registerCopySource(this.editor);
  }

  componentWillUnmount() {
    if (this.copySource) {
      this.copySource.unregister();
    }
  }

  handlePastedText = (_text, html, editorState) => {
    let newState = handleDraftEditorPastedText(html, editorState);
 
    if (newState) {
      this.onChange(newState);
      return true;
    }
 
    return false;
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
    if (command == "backspace" && ! isSelectionAtStart(editorState)) {
      // Don't handle backspace with RichUtils, it deletes list items and
      // breaks lists, unless we're at the very start of the selection
      // so the last remaining bullet can be deleted by backspace.
      return "not-handled";
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

  toggleSpellcheck = () => {
    this.setState(state => {
      const spellcheck = ! state.spellcheck;
      setSpellcheck(spellcheck);

      return { spellcheck };
    });
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
          spellCheck={this.state.spellcheck}
          handlePastedText={this.handlePastedText}
        />

        <ButtonBar
          toggleInlineStyle={this.toggleInlineStyle}
          toggleBlockType={this.toggleBlockType}
          isStyleActive={currentStyle.has.bind(currentStyle)}
          currentBlockType={currentBlockType}
          spellcheck={this.state.spellcheck}
          toggleSpellcheck={this.toggleSpellcheck}
        />
      </div>
    );
  }
}
