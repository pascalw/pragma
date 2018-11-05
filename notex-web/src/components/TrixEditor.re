module EventTarget = Webapi.Dom.EventTarget;

type editorRef = ref(option(Dom.element));

type state = {
  editorRef,
  text: string,
};

type action =
  | UpdateText(string);

let setCursorEnd = [%raw
  {|
  function(element) {
    var editor = element.editor;
    var length = editor.getDocument().toString().length;
    editor.setSelectedRange(length - 1);
  }
|}
];

let setEditorRef = (ref, {ReasonReact.state}) =>
  state.editorRef := Js.Nullable.toOption(ref);

let updateTrixEditorValue = (editor, text) => {
  let domNode = ReactDOMRe.domElementToObj(editor);

  if (domNode##value != text) {
    domNode##value #= text;
    setCursorEnd(. editor) |> ignore;
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
  reducer: (action: action, state: state) =>
    switch (action) {
    | UpdateText(text) => ReasonReact.Update({...state, text})
    },
  willReceiveProps: self => {...self.state, text},
  didUpdate: oldAndNewSelf =>
    if (oldAndNewSelf.oldSelf.state.text !== text) {
      withTrix(oldAndNewSelf.newSelf.state.editorRef, editor =>
        updateTrixEditorValue(editor, text)
      );
    },
  didMount: self =>
    withTrix(
      self.state.editorRef,
      editor => {
        let notifyChange = (event, self) => {
          let target =
            EventRe.target(event)
            |> EventTargetRe.unsafeAsElement
            |> ReactDOMRe.domElementToObj;

          let editorText = target##value;

          if (editorText !== self.ReasonReact.state.text) {
            self.send(UpdateText(editorText));
            onChange(editorText);
          };
        };

        let editorDomNode = editor |> Webapi.Dom.Element.asEventTarget;
        let setEditorValue = _ => {
          updateTrixEditorValue(editor, self.state.text);

          EventTarget.addEventListener(
            "trix-change",
            self.handle(notifyChange),
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
            self.handle(notifyChange),
            editorDomNode,
          );
        });
      },
    ),
  render: self =>
    <div className="trix-wrapper">
      {
        ReasonReact.cloneElement(
          ReactDOMRe.createElement("trix-editor", [||]),
          ~props={
            "class": "trix-content",
            "spellcheck": "false",
            "ref": self.handle(setEditorRef),
          },
          [||],
        )
      }
    </div>,
};
