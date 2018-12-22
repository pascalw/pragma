type state = {editingTitleId: option(string)};

type action =
  | EditTitle(option(string));

let component = ReasonReact.reducerComponent("Notebooks");
let make =
    (
      ~dispatch,
      ~notebooks: list((Data.notebook, int)),
      ~selectedNotebook: option(string),
      ~hidden,
      _children,
    ) => {
  let createNotebook = (send, _) => {
    let notebook = Data.newNotebook();

    let promise =
      Notebooks.create(notebook)
      |> Promises.tapOk(_ => {
           dispatch(NoteManagementContainer.SelectNotebook(notebook));
           send(EditTitle(Some(notebook.id)));
         });

    Db.withPromiseNotification(promise);
  };

  {
    ...component,
    initialState: () => {editingTitleId: None},
    reducer: (action: action, _state: state) =>
      switch (action) {
      | EditTitle(id) => ReasonReact.Update({editingTitleId: id})
      },
    render: self => {
      let renderListItemContent = (notebook: Data.notebook, noteCount: int) =>
        if (Some(notebook.id) == self.state.editingTitleId) {
          <p>
            <NotebookTitleEditor
              value={notebook.title}
              onComplete={
                title => {
                  let updatedNotebook = {...notebook, title};

                  self.send(EditTitle(None));
                  dispatch(
                    NoteManagementContainer.UpdateNotebook(updatedNotebook),
                  );
                }
              }
            />
          </p>;
        } else {
          <ListView.ItemContent title={notebook.title} count=noteCount />;
        };

      <ListView hidden>
        <ListView.ItemContainer>
          {
            Belt.List.map(notebooks, ((notebook, noteCount)) =>
              <ListView.Item
                key={notebook.id}
                selected={Some(notebook.id) == selectedNotebook}
                onClick={
                  _ =>
                    dispatch(
                      NoteManagementContainer.SelectNotebook(notebook),
                    )
                }
                onDoubleClick={_ => self.send(EditTitle(Some(notebook.id)))}
                onLongpress={
                  _ =>
                    if (WindowRe.confirm(
                          "Are you sure you want to delete this notebook?",
                          Webapi.Dom.window,
                        )) {
                      dispatch(
                        NoteManagementContainer.DeleteNotebook(notebook),
                      );
                    }
                }>
                {renderListItemContent(notebook, noteCount)}
              </ListView.Item>
            )
            |> Belt.List.toArray
            |> ReasonReact.array
          }
        </ListView.ItemContainer>
        <ListView.Footer>
          <SyncStateContainer />
          <AddButton onClick={createNotebook(self.send)} />
        </ListView.Footer>
      </ListView>;
    },
  };
};
