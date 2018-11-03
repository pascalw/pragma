let onChangeNoteTextDebounced = send => {
  let timerId: ref(option(Js.Global.timeoutId)) = ref(None);

  (contentBlock, value) => {
    switch (timerId^) {
    | Some(timerId) => Js.Global.clearTimeout(timerId)
    | _ => ()
    };

    timerId :=
      Some(
        Js.Global.setTimeout(
          () =>
            send(
              NoteManagementContainer.UpdateNoteText(contentBlock, value),
            ),
          1000,
        ),
      );
  };
};

let onChange = (send, onChangeNoteText, change) =>
  switch (change) {
  | NoteEditor.Text(contentBlock, value) =>
    onChangeNoteText(contentBlock, value)
  | NoteEditor.Title(note, title) =>
    send(NoteManagementContainer.UpdateNoteTitle(note, title))
  | NoteEditor.ContentBlock(block) =>
    send(NoteManagementContainer.UpdateContentBlock(block))
  };

let component = ReasonReact.statelessComponent("NoteEditor");
let make =
    (
      ~dispatch,
      ~note: option(Data.note),
      ~contentBlocks: list(Data.contentBlock),
      _children,
    ) => {
  ...component,
  render: _self =>
    <>
      {
        switch (note) {
        | None => <NoNoteSelected />
        | Some(note) =>
          <NoteEditor
            note
            contentBlocks
            onChange={
              onChange(dispatch, onChangeNoteTextDebounced(dispatch))
            }
          />
        }
      }
    </>,
};
