let fromNote = noteId => Db.getContentBlocks(noteId);
let get = id => Db.getContentBlock(id);
let add = blocks => Db.addContentBlocks(blocks);

let update = (block, ~sync=true, ()) => {
  let now = Js.Date.fromFloat(Js.Date.now());
  Db.updateContentBlock({...block, updatedAt: now}, ~sync, ());
};

let delete = id => Db.deleteContentBlock(id);

DataSync.setContentBlockSyncedListener(block =>
  Db.updateContentBlock(block, ~sync=false, ())
);
