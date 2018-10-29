type state = {editingTitleId: option(string)};

type action =
  | EditTitle(option(string));

let component = ReasonReact.reducerComponent("Notebooks");
let make =
    (
      ~dispatch,
      ~notebooks: list((Data.notebook, int)),
      ~selectedNotebook: option(string),
      _children,
    ) => {
  let createNotebook = (send, _) => {
    let notebook = Data.newNotebook();

    dispatch(NoteManagementContainer.CreateNotebook(notebook));
    send(EditTitle(Some(notebook.id)));
  };

  {
    ...component,
    initialState: () => {editingTitleId: None},
    reducer: (action: action, _state: state) =>
      switch (action) {
      | EditTitle(id) => ReasonReact.Update({editingTitleId: id})
      },
    render: self => {
      let listFooter =
        <> <AddButton onClick={createNotebook(self.send)} /> </>;
      let listItems =
        Belt.List.map(notebooks, ((notebook, noteCount)) =>
          (
            {
              id: notebook.id,
              title: notebook.title,
              count: Some(noteCount),
              model: notebook,
            }:
              ListView.listItem(Data.notebook)
          )
        );

      let renderNotebookListItemContent =
          (item: ListView.listItem(Data.notebook)) =>
        if (Some(item.model.id) == self.state.editingTitleId) {
          <p>
            <NotebookTitleEditor
              value={item.model.title}
              onComplete={
                title => {
                  let updatedNotebook = {...item.model, title};

                  self.send(EditTitle(None));
                  dispatch(
                    NoteManagementContainer.UpdateNotebook(updatedNotebook),
                  );
                }
              }
            />
          </p>;
        } else {
          ListView.defaultRenderItemContent(item);
        };

      <ListView
        items=listItems
        selectedId=selectedNotebook
        onItemSelected={
          item =>
            dispatch(NoteManagementContainer.SelectNotebook(item.model))
        }
        onItemDoubleClick={
          item => self.send(EditTitle(Some(item.model.id)))
        }
        onItemLongpress={
          item =>
            if (WindowRe.confirm(
                  "Are you sure you want to delete this notebook?",
                  Webapi.Dom.window,
                )) {
              dispatch(NoteManagementContainer.DeleteNotebook(item.model));
            }
        }
        renderItemContent=renderNotebookListItemContent
        renderFooter={() => listFooter}
      />;
    },
  };
};
