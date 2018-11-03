let fromNote = noteId => Db.getContentBlocks(noteId);
let get = id => Db.getContentBlock(id);
let add = blocks => Db.addContentBlocks(blocks);

let update = (block, ~sync=true, ()) => {
  let now = Js.Date.fromFloat(Js.Date.now());
  Db.updateContentBlock({...block, updatedAt: now}, ~sync, ());
};

let updateContentType = (block: Data.contentBlock, newContentType) => {
  let newContent =
    switch (block.content, newContentType) {
    | (Data.TextContent(text), "code") =>
      Data.CodeContent(Utils.htmlToText(text), "")
    | (Data.CodeContent(code, _), "text") => Data.TextContent(code)
    | _ => Js.Exn.raiseError("Unsupported content type change")
    };

  {...block, content: newContent};
};

let updateCodeLanguage = (block: Data.contentBlock, language) =>
  switch (block.content) {
  | Data.CodeContent(code, _) => {
      ...block,
      content: Data.CodeContent(code, language),
    }
  | _ => Js.Exn.raiseError("Unsupported contentblock")
  };

let delete = id => Db.deleteContentBlock(id);

DataSync.setContentBlockSyncedListener(block =>
  Db.updateContentBlock(block, ~sync=false, ())
);
