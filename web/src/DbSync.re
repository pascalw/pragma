open Belt;

let upsertContentBlock = (contentBlock: Data.contentBlock) =>
  ContentBlocks.get(contentBlock.id)
  |> Repromise.wait(storedContentBlock =>
       switch (storedContentBlock) {
       | None => ContentBlocks.add(contentBlock) |> ignore
       | Some(_contentBlock) =>
         ContentBlocks.update(contentBlock, ~sync=false, ()) |> ignore
       }
     );

let upsertNote = (note: Data.note) =>
  Notes.get(note.id)
  |> Repromise.wait(storedNote =>
       switch (storedNote) {
       | None => Notes.add(note) |> ignore
       | Some(_note) => Notes.update(note, ~sync=false, ()) |> ignore
       }
     );

let upsertNotebook = (notebook: Data.notebook) =>
  Notebooks.get(notebook.id)
  |> Repromise.wait(storedNotebook =>
       switch (storedNotebook) {
       | None => Notebooks.add(notebook) |> ignore
       | Some(_note) => Notebooks.update(notebook, ~sync=false, ()) |> ignore
       }
     );

let run = () =>
  Db.getRevision()
  |> Repromise.andThen(Api.fetchChanges)
  |> Repromise.wait((result: Belt.Result.t(Api.apiResponse, _)) =>
       switch (result) {
       | Result.Ok(result) =>
         Db.withNotification(() => {
           let revision = result.revision;
           Db.insertRevision(revision) |> ignore;

           /* FIXME: wait on all promises */
           result.changes.notebooks->List.forEach(upsertNotebook);
           result.changes.notes->List.forEach(upsertNote);
           result.changes.contentBlocks->List.forEach(upsertContentBlock);

           List.forEach(result.deletions, deletedResource =>
             switch (deletedResource.type_) {
             | "notebook" =>
               Notebooks.delete(deletedResource.id, ~sync=false, ())
             | "note" => Notes.delete(deletedResource.id, ~sync=false, ())
             | "contentBlock" => ContentBlocks.delete(deletedResource.id)
             | type_ =>
               Js.Exn.raiseError("Unsupported deletion type: " ++ type_)
             }
           );
         })
       | Result.Error(reason) =>
         Js.Console.error2("Failed to fetch changes", reason)
       }
     );
