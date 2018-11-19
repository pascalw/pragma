[@bs.module "./RichTextEditor.js"]
external richTextEditorComponent: ReasonReact.reactClass = "RichTextEditor";

module RichTextEditorProps = {
  [@bs.deriving abstract]
  type t('a) = {
    value: 'a,
    onChange: RichText.t => unit,
    onShiftEnter: unit => unit,
    onDeleteIntent: unit => unit
  };
};

let make = (~value: RichText.t, ~onChange, ~onShiftEnter, ~onDeleteIntent, children) =>
  ReasonReact.wrapJsForReason(
    ~reactClass=richTextEditorComponent,
    ~props=RichTextEditorProps.t(~value, ~onChange, ~onShiftEnter, ~onDeleteIntent),
    children,
  );
