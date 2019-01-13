import React from "react";
import ReactDOMServer from "react-dom/server";
import { EditorState, Modifier } from "draft-js";
import {
  convertToHTML as rawConvertToHTML,
  convertFromHTML as rawConvertFromHTML
} from "draft-convert";
import { CHECKABLE_LIST_ITEM_TYPE } from "./constants";

const convertToHTML = rawConvertToHTML({
  styleToHTML: style => {
    switch(style) {
      case "STRIKETHROUGH": 
        return <del />;
      case "MARK":
        return <mark />;
    }
  },
  blockToHTML: block => {
    if(block.type === CHECKABLE_LIST_ITEM_TYPE) {
      const checked = !! block.data.checked;
      const input = <input type="checkbox" defaultChecked={checked} />;

      return {
        start: "<li>" + ReactDOMServer.renderToStaticMarkup(input),
        end: "</li>",
        nest: <ul/>
      }
    };
  },
  entityToHTML: (entity, originalText) => {
    if (entity.type === "LINK") {
      return <a href={entity.data.url}>{originalText}</a>;
    }
    return originalText;
  }
});

const convertFromHTML = rawConvertFromHTML({
  htmlToStyle: (nodeName, _node, currentStyle) => {
    switch(nodeName) {
      case "mark":
        return currentStyle.add("MARK");
      case "del":
        return currentStyle.add("STRIKETHROUGH");
      default:
        return currentStyle;
    }
  },
  htmlToEntity: (nodeName, node, createEntity) => {
    if (nodeName === "a") {
      return createLinkEntity(createEntity, node.href);
    }
  },
  htmlToBlock: (nodeName, node) => {
    if (nodeName === "li") {
      const checkbox = node.querySelector("input[type=checkbox]");
      if(checkbox) {
        return {
            type: CHECKABLE_LIST_ITEM_TYPE,
            data: { checked: checkbox.checked }
        };
      }
    }
  }
});

export const editorStateToHtml = editorState => {
  return convertToHTML(editorState.getCurrentContent());
};

export const htmlToEditorState = html => {
  return EditorState.createWithContent(convertFromHTML(html));
};

export const isSelectionAtStart = editorState => {
  const selectionState = editorState.getSelection();
  const firstBlock = editorState
    .getCurrentContent()
    .getBlockMap()
    .first();

  return (
    selectionState.getFocusKey() === firstBlock.getKey() &&
    selectionState.getStartOffset() == 0
  );
};

const createLinkEntity = (createEntity, url) => {
  return createEntity(
    "LINK",
    "MUTABLE",
    { url }
  );
};

export const applyLinkToSelection = (editorState, url) => {
  const contentState = editorState.getCurrentContent();

  let newContentState = createLinkEntity(contentState.createEntity.bind(contentState), url);
  const entityKey = newContentState.getLastCreatedEntityKey();
  newContentState = Modifier.applyEntity(newContentState, editorState.getSelection(), entityKey);

  let newEditorState = EditorState.push(
    editorState,
    newContentState,
    "apply-entity"
  );

  return newEditorState;
};

export const createLinkedText = (editorState, url, text) => {
  const contentState = editorState.getCurrentContent();

  let newContentState = createLinkEntity(contentState.createEntity.bind(contentState), url);
  const entityKey = newContentState.getLastCreatedEntityKey();
  newContentState = Modifier.insertText(newContentState, editorState.getSelection(), text, null, entityKey);

  let newEditorState = EditorState.push(
    editorState,
    newContentState,
    "insert-characters"
  );

  let newSelection = newContentState.getSelectionAfter();
  newEditorState = EditorState.forceSelection(newEditorState, newSelection);

  return newEditorState;
};
