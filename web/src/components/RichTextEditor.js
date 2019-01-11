import React from "react";
import {
  Editor,
  RichUtils,
  CompositeDecorator,
  EditorState,
  DefaultDraftBlockRenderMap
} from "draft-js";
import {
  registerCopySource,
  handleDraftEditorPastedText
} from "draftjs-conductor";
import { handleReturnInList } from "../draft-js/list-behavior";
import {
  isSelectionAtStart,
  applyLinkToSelection,
  createLinkedText,
} from "../draft-js/utils";
import { CHECKABLE_LIST_ITEM_TYPE } from "../draft-js/constants";
import classNames from "classnames/bind";
import styles from "./RichTextEditor.scss";
import { jsComponent as ButtonBar } from "./RichTextButtonBar.bs";
import { jsComponent as LinkComponent } from "./editor/Link.bs";
import { isUrl } from "../support/Utils.bs";
import CheckableListItem, {
  listItemStyle,
  toggleChecked,
  onTab as onCheckableListItemTab,
  blockRenderMap as listItemBlockRenderMap
} from "./RichTextEditor/CheckableListItem";

const maxListDepth = 4;

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

const findLinkEntities = (contentBlock, callback, contentState) => {
  contentBlock.findEntityRanges(character => {
    const entityKey = character.getEntity();
    return (
      entityKey !== null &&
      contentState.getEntity(entityKey).getType() === "LINK"
    );
  }, callback);
};
const Link = props => {
  const { url } = props.contentState.getEntity(props.entityKey).getData();
  return <LinkComponent url={url}>{props.children}</LinkComponent>;
};

const decorator = new CompositeDecorator([
  {
    strategy: findLinkEntities,
    component: Link
  }
]);

const editorStateFromValue = value => {
  return EditorState.set(value, { decorator });
};

export class RichTextEditor extends React.Component {
  constructor(props) {
    super(props);

    this.focus = () => this.editor.focus();
    this.blur = () => this.editor.blur();

    this.isFocused = () => {
      return this.state.editorState.getSelection().getHasFocus();
    };

    this.state = {
      editorState: editorStateFromValue(props.value),
      spellcheck: spellcheckEnabled()
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

  blockRendererFn = block => {
    if (block.getType() === CHECKABLE_LIST_ITEM_TYPE) {
      return {
        component: CheckableListItem,
        props: {
          onChangeChecked: () =>
            this.onChange(toggleChecked(this.state.editorState, block)),
          checked: !!block.getData().get("checked")
        }
      };
    }
  };

  blockStyleFn(block) {
    if (block.getType() === CHECKABLE_LIST_ITEM_TYPE) {
      return listItemStyle;
    }
  }

  handlePastedText = (text, html, editorState) => {
    if (isUrl(text)) {
      const url = text;
      let newEditorState;

      if (editorState.getSelection().isCollapsed()) {
        newEditorState = createLinkedText(editorState, url, url);
      } else {
        newEditorState = applyLinkToSelection(editorState, url);
      }

      this.onChange(newEditorState);
      return true;
    }

    let newState = handleDraftEditorPastedText(html, editorState);

    if (newState) {
      this.onChange(newState);
      return true;
    }

    return false;
  };

  onTab = e => {
    let newState = onCheckableListItemTab(
      e,
      this.state.editorState,
      maxListDepth
    );
    newState = RichUtils.onTab(e, newState, maxListDepth);

    if (newState) {
      this.onChange(newState);
    }
  };

  onChange = editorState => {
    if (
      editorState.getCurrentContent() !==
      this.state.editorState.getCurrentContent()
    ) {
      this.props.onChange(editorState);
    }

    this.setState({ editorState });
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
    if (command == "backspace" && !isSelectionAtStart(editorState)) {
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
    this.onChange(RichUtils.toggleBlockType(this.state.editorState, blockType));
  };

  toggleInlineStyle = inlineStyle => {
    this.onChange(
      RichUtils.toggleInlineStyle(this.state.editorState, inlineStyle)
    );
  };

  toggleSpellcheck = () => {
    this.setState(state => {
      const spellcheck = !state.spellcheck;
      setSpellcheck(spellcheck);

      return { spellcheck };
    });
  };

  currentBlockType = () => {
    const editorState = this.state.editorState;

    const selection = editorState.getSelection();

    return editorState
      .getCurrentContent()
      .getBlockForKey(selection.getStartKey())
      .getType();
  };

  currentStyle = () => {
    return this.state.editorState.getCurrentInlineStyle();
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
          editorState={this.state.editorState}
          onChange={this.onChange}
          onTab={this.onTab}
          customStyleMap={styleMap}
          handleReturn={this.handleReturn}
          handleKeyCommand={this.handleKeyCommand}
          spellCheck={this.state.spellcheck}
          handlePastedText={this.handlePastedText}
          blockRendererFn={this.blockRendererFn}
          blockRenderMap={DefaultDraftBlockRenderMap.merge(
            listItemBlockRenderMap
          )}
          blockStyleFn={this.blockStyleFn}
          onEscape={_e => this.blur()}
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
