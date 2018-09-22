let insertOrUpdateContentBlock = (contentBlock: Api.contentBlock) =>
  Db.getContentBlock(contentBlock.id)
  ->Future.get(storedContentBlock =>
      switch (storedContentBlock) {
      | None =>
        Db.addContentBlocks([
          {
            id: contentBlock.id,
            noteId: contentBlock.noteId,
            content:
              switch (contentBlock.content) {
              | Api.TextContent(text) => TextContent(text)
              | Api.CodeContent(code, language) =>
                CodeContent(code, language)
              },
          },
        ])
        |> ignore
      | Some(_contentBlock) =>
        Db.updateContentBlock(
          {
            id: contentBlock.id,
            noteId: contentBlock.noteId,
            content:
              switch (contentBlock.content) {
              | Api.TextContent(text) => TextContent(text)
              | Api.CodeContent(code, language) =>
                CodeContent(code, language)
              },
          },
          ~sync=false,
          (),
        )
        |> ignore
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

        apiResult.changes.notebooks
        ->(
            Belt.List.forEach(notebook =>
              Db.addNotebooks([
                {
                  id: notebook.id,
                  name: notebook.name,
                  createdAt: notebook.createdAt,
                },
              ])
            )
          );
        apiResult.changes.notes
        ->(
            Belt.List.forEach(note =>
              Db.addNotes([
                {
                  id: note.id,
                  notebookId: note.notebookId,
                  title: note.title,
                  tags: note.tags,
                  createdAt: note.createdAt,
                  updatedAt: note.updatedAt,
                },
              ])
            )
          );

        apiResult.changes.contentBlocks
        ->Belt.List.forEach(insertOrUpdateContentBlock);
      })
    );
