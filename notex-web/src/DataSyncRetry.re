let futureToPromise = future =>
  Js.Promise.make((~resolve, ~reject as _) =>
    Future.get(future, value => resolve(. value))
  );

let optionToResult = option =>
  switch (option) {
  | None => Belt.Result.Error()
  | Some(value) => Belt.Result.Ok(value)
  };

let mapSome = (future, fn) =>
  Future.map(future, value => Belt.Option.map(value, fn));

let getPendingChanges = () => {
  let changeIds = DataSyncPersistence.getStoredChangeIds();

  let promises =
    Belt.List.map(
      changeIds,
      changeId => {
        let change =
          switch (Js.String.split(":", changeId)) {
          | [|"contentBlock", "updated", id|] =>
            Db.getContentBlock(id)
            ->mapSome(cb => DataSync.ContentBlockUpdated(cb))

          | [|"contentBlock", "created", id|] =>
            Db.getContentBlock(id)
            ->mapSome(cb => DataSync.ContentBlockCreated(cb))

          | [|"note", "created", id|] =>
            Db.getNote(id)->mapSome(note => DataSync.NoteCreated(note))

          | [|"note", "updated", id|] =>
            Db.getNote(id)->mapSome(note => DataSync.NoteUpdated(note))

          | [|"note", "deleted", id|] =>
            Future.value(Some(DataSync.NoteDeleted(id)))

          | [|"notebook", "created", id|] =>
            Db.getNotebook(id)
            ->mapSome(notebook => DataSync.NotebookCreated(notebook))

          | [|"notebook", "updated", id|] =>
            Db.getNotebook(id)
            ->mapSome(notebook => DataSync.NotebookUpdated(notebook))

          | [|"notebook", "deleted", id|] =>
            Future.value(Some(DataSync.NotebookDeleted(id)))

          | _ =>
            Js.Console.error2("Unknown change id:", changeId);
            Future.value(None);
          };

        change
        ->mapSome(change => {DataSync.id: changeId, change})
        ->futureToPromise;
      },
    )
    ->Array.of_list;

  promises
  ->Js.Promise.all
  ->FutureJs.fromPromise(Js.String.make)
  ->Future.map(result => Belt.Result.getWithDefault(result, [||]))
  ->Future.map(Array.to_list)
  ->Future.map(result => Belt.List.keepMap(result, item => item));
};
