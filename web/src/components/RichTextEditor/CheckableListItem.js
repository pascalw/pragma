// based on https://github.com/sugarshin/draft-js-checkable-list-item

import React, { Component } from "react";
import { EditorBlock } from "draft-js";
import {
  CheckableListItemUtils,
  blockRenderMap as blockRenderMap_
} from "draft-js-checkable-list-item";

import styles from "./CheckableListItem.scss";

export const listItemStyle = styles.listItem;

export const toggleChecked = (editorState, block) =>
  CheckableListItemUtils.toggleChecked(editorState, block);

export const onTab = (e, editorState, maxListDepth) =>
  CheckableListItemUtils.onTab(e, editorState, maxListDepth);

export const blockRenderMap = blockRenderMap_;

export default class CheckableListItem extends Component {
  render() {
    const {
      offsetKey,
      blockProps: { onChangeChecked, checked }
    } = this.props;
    return (
      <div data-offset-key={offsetKey}>
        <div
          className={styles.checkbox}
          contentEditable={false}
          suppressContentEditableWarning
        >
          <input type="checkbox" checked={checked} onChange={onChangeChecked} />
        </div>
        <div className={styles.text}>
          <EditorBlock {...this.props} />
        </div>
      </div>
    );
  }
}
