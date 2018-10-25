open Belt;

let upsertContentBlock = (contentBlock: Data.contentBlock) =>
  ContentBlocks.get(contentBlock.id)
  ->Future.get(storedContentBlock =>
      switch (storedContentBlock) {
      | None => ContentBlocks.add([contentBlock]) |> ignore
      | Some(_contentBlock) =>
        ContentBlocks.update(contentBlock, ~sync=false, ()) |> ignore
      }
    );

let upsertNote = (note: Data.note) =>
  Notes.get(note.id)
  ->Future.get(storedNote =>
      switch (storedNote) {
      | None => Notes.add([note]) |> ignore
      | Some(_note) => Notes.update(note, ~sync=false, ()) |> ignore
      }
    );

let upsertNotebook = (notebook: Data.notebook) =>
  Notebooks.get(notebook.id)
  ->Future.get(storedNotebook =>
      switch (storedNotebook) {
      | None => Notebooks.add([notebook]) |> ignore
      | Some(_note) => Notebooks.update(notebook, ~sync=false, ()) |> ignore
      }
    );

let run = () =>
  Db.getRevision()
  ->Future.flatMap(Api.fetchChanges)
  ->(
      Future.get(result => {
        let apiResult = Result.getExn(result);

        Db.withNotification(() => {
          let revision = apiResult.revision;
          Db.insertRevision(revision) |> ignore;

          apiResult.changes.notebooks->List.forEach(upsertNotebook);
          apiResult.changes.notes->List.forEach(upsertNote);
          apiResult.changes.contentBlocks->List.forEach(upsertContentBlock);

          List.forEach(apiResult.deletions, deletedResource =>
            switch (deletedResource.type_) {
            | "notebook" =>
              Notebooks.delete(deletedResource.id, ~sync=false, ())
            | "note" => Notes.delete(deletedResource.id, ~sync=false, ())
            | "contentBlock" => ContentBlocks.delete(deletedResource.id)
            | type_ =>
              Js.Exn.raiseError("Unsupported deletion type: " ++ type_)
            }
          );
        });
      })
    );
