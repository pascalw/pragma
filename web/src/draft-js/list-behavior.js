import {
  getSelectedBlock,
  decreaseBlockDepth,
  changeBlockType
} from "draft-js-list-depth-plugin/src/utils";

const handleReturnForListItem = (editorState, block) => {
  const depth = block.getDepth();
  if (depth > 0) {
    return decreaseBlockDepth(editorState, block);
  }
  return changeBlockType(editorState);
};

export const handleReturnInList = (editorState) => {
  const block = getSelectedBlock(editorState);

  if (isEmptyListItem(block)) {
    return handleReturnForListItem(editorState, block);
  }

  return null;
};

const isEmptyListItem = block => {
  const text = block.getText();
  const hasEmptyText = text.length === 0;
  const blockType = block.getType();
  const isListItemBlock =
    blockType === "unordered-list-item" || blockType === "ordered-list-item" || blockType == "checkable-list-item";

  return isListItemBlock && hasEmptyText;
};
