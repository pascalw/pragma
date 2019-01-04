let optionToResult = option =>
  switch (option) {
  | None => Belt.Result.Error()
  | Some(value) => Belt.Result.Ok(value)
  };

let getPendingChanges = () => {
  let changeIds = DataSyncPersistence.getStoredChangeIds();

  let promises =
    Belt.List.map(
      changeIds,
      changeId => {
        let change =
          switch (Js.String.split(":", changeId)) {
          | [|"contentBlock", "updated", id|] =>
            ContentBlocks.get(id) |> Promises.mapSome(cb => DataSync.ContentBlockUpdated(cb))

          | [|"contentBlock", "created", id|] =>
            ContentBlocks.get(id) |> Promises.mapSome(cb => DataSync.ContentBlockCreated(cb))

          | [|"note", "created", id|] =>
            Notes.get(id) |> Promises.mapSome(note => DataSync.NoteCreated(note))

          | [|"note", "updated", id|] =>
            Notes.get(id) |> Promises.mapSome(note => DataSync.NoteUpdated(note))

          | [|"note", "deleted", id|] => Repromise.resolved(Some(DataSync.NoteDeleted(id)))

          | [|"notebook", "created", id|] =>
            Notebooks.get(id) |> Promises.mapSome(notebook => DataSync.NotebookCreated(notebook))

          | [|"notebook", "updated", id|] =>
            Notebooks.get(id) |> Promises.mapSome(notebook => DataSync.NotebookUpdated(notebook))

          | [|"notebook", "deleted", id|] =>
            Repromise.resolved(Some(DataSync.NotebookDeleted(id)))

          | _ =>
            Js.Console.error2("Unknown change id:", changeId);
            Repromise.resolved(None);
          };

        Promises.mapSome(change => {DataSync.id: changeId, change}, change);
      },
    );

  promises |> Repromise.all |> Repromise.map(result => Belt.List.keepMap(result, item => item));
};
