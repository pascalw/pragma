[@bs.module] external styles: Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let isAuthedUser = () => Auth.getToken() |> Belt.Option.isSome;

type state =
  | ();
type action =
  | Render;

let component = ReasonReact.reducerComponent("App");
let make = _children => {
  ...component,
  initialState: () => (),
  reducer: (_action: action, _state: state) => ReasonReact.Update(),
  render: self => {
    let editingNote = (notes: list(Data.note), selectedNoteId) =>
      Belt.Option.flatMap(selectedNoteId, selectedNoteId =>
        Utils.find(notes, n => n.id == selectedNoteId)
      );

    isAuthedUser() ?
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
      </> :
      <OnboardingContainer onCompleted={() => self.send(Render)} />;
  },
};
