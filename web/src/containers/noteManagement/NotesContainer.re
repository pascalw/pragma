let component = ReasonReact.statelessComponent("Notes");
let make =
    (
      ~dispatch,
      ~notes: list(Data.note),
      ~selectedNote,
      ~selectedNotebook,
      ~hidden,
      _children,
    ) => {
  ...component,
  render: _self => {
    let formatDate = date => Utils.formatDate(date, "D MMMM YYYY");

    <ListView minWidth="250px" hidden>
      <ListView.ItemContainer>
        {
          Belt.List.map(notes, note =>
            <ListView.Item
              key={note.id}
              selected={Some(note.id) == selectedNote}
              onClick={
                _ => dispatch(NoteManagementContainer.SelectNote(note.id))
              }
              onLongpress={
                _ =>
                  if (WindowRe.confirm(
                        "Are you sure you want to delete this note?",
                        Webapi.Dom.window,
                      )) {
                    dispatch(NoteManagementContainer.DeleteNote(note));
                  }
              }>
              <p>
                {ReasonReact.string(note.title)}
                <br />
                <small>
                  {ReasonReact.string(note.updatedAt |> formatDate)}
                </small>
              </p>
            </ListView.Item>
          )
          |> Belt.List.toArray
          |> ReasonReact.array
        }
      </ListView.ItemContainer>
      <ListView.Footer>
        {
          switch (selectedNotebook) {
          | None => ReasonReact.null
          | Some(_) =>
            <AddButton
              onClick=(_ => dispatch(NoteManagementContainer.CreateNote))
            />
          }
        }
      </ListView.Footer>
    </ListView>;
  },
};
