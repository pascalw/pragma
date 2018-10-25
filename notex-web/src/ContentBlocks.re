let fromNote = noteId => Db.getContentBlocks(noteId);
let get = id => Db.getContentBlock(id);
let add = blocks => Db.addContentBlocks(blocks);
let update = (block, ~sync=true, ()) =>
  Db.updateContentBlock(block, ~sync, ());
let delete = id => Db.deleteContentBlock(id);

DataSync.setContentBlockSyncedListener(block =>
  Db.updateContentBlock(block, ~sync=false, ())
);
