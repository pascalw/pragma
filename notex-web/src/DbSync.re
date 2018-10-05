let upsertContentBlock = (contentBlock: Data.contentBlock) =>
  Db.getContentBlock(contentBlock.id)
  ->Future.get(storedContentBlock =>
      switch (storedContentBlock) {
      | None => Db.addContentBlocks([contentBlock]) |> ignore
      | Some(_contentBlock) =>
        Db.updateContentBlock(contentBlock, ~sync=false, ()) |> ignore
      }
    );

let upsertNote = (note: Data.note) =>
  Db.getNote(note.id)
  ->Future.get(storedNote =>
      switch (storedNote) {
      | None => Db.addNotes([note]) |> ignore
      | Some(_note) => Db.updateNote(note, ~sync=false, ()) |> ignore
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

        apiResult.changes.notes->Belt.List.forEach(upsertNote);

        apiResult.changes.contentBlocks
        ->Belt.List.forEach(upsertContentBlock);
      })
    );
