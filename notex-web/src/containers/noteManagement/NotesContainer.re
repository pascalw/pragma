let component = ReasonReact.statelessComponent("Notes");
let make =
    (
      ~dispatch,
      ~notes: list(Data.note),
      ~selectedNote,
      ~selectedNotebook,
      _children,
    ) => {
  ...component,
  render: _self => {
    let renderFooter =
      switch (selectedNotebook) {
      | None => (() => ReasonReact.null)
      | Some(_) => (
          () =>
            <>
              <AddButton
                onClick={_ => dispatch(NoteManagementContainer.CreateNote)}
              />
            </>
        )
      };

    let listItems =
      Belt.List.map(notes, note =>
        (
          {id: note.id, title: note.title, count: None, model: note}:
            ListView.listItem(Data.note)
        )
      );

    let formatDate = date => DateFns.format("D MMMM YYYY", date);
    let renderNoteListItemContent = (item: ListView.listItem(Data.note)) =>
      <p>
        {ReasonReact.string(item.model.title)}
        <br />
        <small>
          {ReasonReact.string(item.model.updatedAt |> formatDate)}
        </small>
      </p>;

    <ListView
      minWidth="250px"
      items=listItems
      selectedId=selectedNote
      onItemSelected={
        item => dispatch(NoteManagementContainer.SelectNote(item.model.id))
      }
      onItemLongpress={
        item =>
          if (WindowRe.confirm(
                "Are you sure you want to delete this note?",
                Webapi.Dom.window,
              )) {
            dispatch(NoteManagementContainer.DeleteNote(item.model));
          }
      }
      renderItemContent=renderNoteListItemContent
      renderFooter
    />;
  },
};
