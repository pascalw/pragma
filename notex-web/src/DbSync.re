open Belt;

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

let upsertNotebook = (notebook: Data.notebook) =>
  Db.getNotebook(notebook.id)
  ->Future.get(storedNotebook =>
      switch (storedNotebook) {
      | None => Db.addNotebooks([notebook]) |> ignore
      | Some(_note) => Db.updateNotebook(notebook, ~sync=false, ()) |> ignore
      }
    );

let run = () =>
  Db.getRevision()
  ->(Future.flatMap(Api.fetchChanges))
  ->(
      Future.get(result => {
        let apiResult = Result.getExn(result);

        let revision = apiResult.revision;
        Db.insertRevision(revision) |> ignore;

        apiResult.changes.notebooks->List.forEach(upsertNotebook);
        apiResult.changes.notes->List.forEach(upsertNote);
        apiResult.changes.contentBlocks->List.forEach(upsertContentBlock);
      })
    );
