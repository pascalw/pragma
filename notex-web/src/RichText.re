type t;

[@bs.module "./draft-js/utils"]
external htmlToEditorState: string => t = "";

[@bs.module "./draft-js/utils"]
external editorStateToHtml: t => string = "";

let toString = editorStateToHtml;
let fromString = htmlToEditorState;

let create = () => fromString("");
