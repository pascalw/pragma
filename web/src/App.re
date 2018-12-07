[@bs.module] external styles: Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let isAuthedUser = () => Auth.getToken() |> Belt.Option.isSome;
type viewMode =
  | SingleColumn
  | TwoColumn
  | ThreeColumn;

type state = {viewMode};

type action =
  | Render
  | SetViewMode(viewMode);

let component = ReasonReact.reducerComponent("App");
let make = _children => {
  ...component,
  initialState: () => {viewMode: ThreeColumn},
  didMount: self => {
    Mousetrap.bind("\\ 1", _e => self.send(SetViewMode(SingleColumn)));

    Mousetrap.bind("\\ 2", _e => self.send(SetViewMode(TwoColumn)));

    Mousetrap.bind("\\ 3", _e => self.send(SetViewMode(ThreeColumn)));

    self.onUnmount(() => {
      Mousetrap.unbind("v 1");
      Mousetrap.unbind("v 2");
      Mousetrap.unbind("v 3");
    });
  },
  reducer: (action: action, state: state) =>
    switch (action) {
    | SetViewMode(viewMode) => ReasonReact.Update({viewMode: viewMode})
    | Render => ReasonReact.Update(state)
    },
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
                       hidden={self.state.viewMode != ThreeColumn}
                       dispatch
                     />
                     <NotesContainer
                       notes={state.notes}
                       selectedNote={state.selectedNote}
                       selectedNotebook={state.selectedNotebook}
                       hidden={self.state.viewMode == SingleColumn}
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
