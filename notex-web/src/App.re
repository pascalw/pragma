[@bs.module] external styles: Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let component = ReasonReact.statelessComponent("App");
let make = _children => {
  ...component,
  render: _self => {
    let editingNote = (notes: list(Data.note), selectedNoteId) =>
      Belt.Option.flatMap(selectedNoteId, selectedNoteId =>
        Utils.find(notes, n => n.id == selectedNoteId)
      );

    <>
      <NoteManagementContainer>
        ...{
             (state, dispatch) =>
               <main className={style("main")}>
                 <div className={style("columns")}>
                   <NotebooksContainer
                     notebooks={state.notebooks}
                     selectedNotebook={state.selectedNotebook}
                     dispatch
                   />
                   <NotesContainer
                     notes={state.notes}
                     selectedNote={state.selectedNote}
                     selectedNotebook={state.selectedNotebook}
                     dispatch
                   />
                   <NoteEditorContainer
                     note={editingNote(state.notes, state.selectedNote)}
                     contentBlocks={state.contentBlocks}
                     dispatch
                   />
                 </div>
               </main>
           }
      </NoteManagementContainer>
      <Toast.Container />
    </>;
  },
};
