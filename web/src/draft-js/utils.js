import React from "react";
import { EditorState, createEntity } from "draft-js";
import {
  convertToHTML as rawConvertToHTML,
  convertFromHTML as rawConvertFromHTML
} from "draft-convert";

const convertToHTML = rawConvertToHTML({
  styleToHTML: style => {
    if (style === "STRIKETHROUGH") {
      return <del />;
    }
  },
  entityToHTML: (entity, originalText) => {
    if (entity.type === "LINK") {
      return <a href={entity.data.url}>{originalText}</a>;
    }
    return originalText;
  }
});

const convertFromHTML = rawConvertFromHTML({
  htmlToEntity: (nodeName, node, createEntity) => {
    if (nodeName === "a") {
      return createEntity("LINK", "MUTABLE", { url: node.href });
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
