[@bs.module "./RichTextEditor.js"]
external richTextEditorComponent: ReasonReact.reactClass = "RichTextEditor";

module RichTextEditorProps = {
  [@bs.deriving abstract]
  type t('a) = {
    value: 'a,
    onChange: RichText.t => unit,
  };
};

let make = (~value: RichText.t, ~onChange, children) =>
  ReasonReact.wrapJsForReason(
    ~reactClass=richTextEditorComponent,
    ~props=RichTextEditorProps.t(~value, ~onChange),
    children,
  );
