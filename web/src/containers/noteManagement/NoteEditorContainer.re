let onChange = (send, change) =>
  switch (change) {
  | NoteEditor.Content(contentBlock, value) =>
    send(NoteManagementContainer.UpdateNoteText(contentBlock, value))
  | NoteEditor.Title(note, title) => send(NoteManagementContainer.UpdateNoteTitle(note, title))
  | NoteEditor.ContentBlock(block) => send(NoteManagementContainer.UpdateContentBlock(block))
  };

let component = ReasonReact.statelessComponent("NoteEditor");
let make =
    (~dispatch, ~note: option(Data.note), ~contentBlocks: list(Data.contentBlock), _children) => {
  ...component,
  render: _self =>
    <>
      {switch (note) {
       | None => <NoNoteSelected />
       | Some(note) => <NoteEditor note contentBlocks onChange={onChange(dispatch)} />
       }}
    </>,
};
