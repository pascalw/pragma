let insertOrUpdateContentBlock = (contentBlock: Data.contentBlock) =>
  Db.getContentBlock(contentBlock.id)
  ->Future.get(storedContentBlock =>
      switch (storedContentBlock) {
      | None => Db.addContentBlocks([contentBlock]) |> ignore
      | Some(_contentBlock) =>
        Db.updateContentBlock(contentBlock, ~sync=false, ()) |> ignore
      }
    );

let run = () =>
  Db.getRevision()
  ->(Future.flatMap(Api.fetchChanges))
  ->(
      Future.get(result => {
        let apiResult = Belt.Result.getExn(result);

        let revision = apiResult.revision;
        Db.insertRevision(revision) |> ignore;

        Db.addNotebooks(apiResult.changes.notebooks) |> ignore;
        Db.addNotes(apiResult.changes.notes) |> ignore;

        apiResult.changes.contentBlocks
        ->Belt.List.forEach(insertOrUpdateContentBlock);
      })
    );
