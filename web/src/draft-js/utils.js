import React from "react";
import {EditorState} from "draft-js";
import { convertToHTML as rawConvertToHTML, convertFromHTML } from "draft-convert";

const convertToHTML = rawConvertToHTML({
  styleToHTML: (style) => {
    if (style === "STRIKETHROUGH") {
      return <del />;
    }
  }
});

export const editorStateToHtml = editorState => {
  return convertToHTML(editorState.getCurrentContent());
};

export const htmlToEditorState = html => {
  return EditorState.createWithContent(convertFromHTML(html));
};
