module EventTarget = Webapi.Dom.EventTarget;

type editorRef = ref(option(Dom.element));

type state = {
  editorRef,
  text: string,
};

let setEditorRef = (ref, {ReasonReact.state}) =>
  state.editorRef := Js.Nullable.toOption(ref);

let updateTrixEditorValue = (editor, text) => {
  let domNode = ReactDOMRe.domElementToObj(editor);

  if (domNode##value != text) {
    domNode##value#=text;
  };
};

let withTrix = (editorRef, fn) =>
  switch (editorRef^) {
  | Some(editor) => fn(editor)
  | _ => ()
  };

let component = ReasonReact.reducerComponent("TrixEditor");

let make = (~text: string, ~onChange, _children) => {
  ...component,
  initialState: () => ({editorRef: ref(None), text}: state),
  reducer: (_state: state, _action) => ReasonReact.NoUpdate,
  didMount: self =>
    withTrix(
      self.state.editorRef,
      editor => {
        let notifyChange = event => {
          let target =
            EventRe.target(event)
            |> EventTargetRe.unsafeAsElement
            |> ReactDOMRe.domElementToObj;

          onChange(target##value);
        };

        let editorDomNode = editor |> Webapi.Dom.Element.asEventTarget;
        let setEditorValue = _ => {
          updateTrixEditorValue(editor, self.state.text);

          EventTarget.addEventListener(
            "trix-change",
            notifyChange,
            editorDomNode,
          );
        };

        EventTarget.addEventListener(
          "trix-initialize",
          setEditorValue,
          editorDomNode,
        );

        self.onUnmount(() => {
          EventTarget.removeEventListener(
            "trix-initialize",
            setEditorValue,
            editorDomNode,
          );

          EventTarget.removeEventListener(
            "trix-change",
            notifyChange,
            editorDomNode,
          );
        });
      },
    ),
  render: self =>
    <div>
      (
        ReasonReact.cloneElement(
          ReactDOMRe.createElement("trix-editor", [||]),
          ~props={"class": "trix-content", "ref": self.handle(setEditorRef)},
          [||],
        )
      )
    </div>,
};