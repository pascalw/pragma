open UiTypes;

let component = ReasonReact.statelessComponent("Notes");
let make =
    (
      ~dispatch,
      ~notes: list(Data.note),
      ~selectedNote,
      ~selectedNoteCollection: option(NoteCollection.t),
      ~hidden,
      _children,
    ) => {
  ...component,
  render: _self => {
    let formatDate = date => Utils.formatDate(date, "D MMMM YYYY");

    <ListView minWidth="250px" hidden>
      <ListView.ItemContainer>
        {Belt.List.map(notes, note =>
           <ListView.Item
             key={note.id}
             selected={Some(note.id) == selectedNote}
             onClick={_ => dispatch(NoteManagementContainer.SelectNote(note.id))}
             onLongpress={_ =>
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
               <small> {ReasonReact.string(note.updatedAt |> formatDate)} </small>
             </p>
           </ListView.Item>
         )
         |> Belt.List.toArray
         |> ReasonReact.array}
      </ListView.ItemContainer>
      <ListView.Footer>
        {switch (selectedNoteCollection) {
         | Some(collection) when NoteCollection.supportsNoteCreation(collection) =>
           <AddButton onClick={_ => dispatch(NoteManagementContainer.CreateNote)} />
         | _ => ReasonReact.null
         }}
      </ListView.Footer>
    </ListView>;
  },
};
