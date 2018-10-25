let fromNotebook = notebookId => Db.getNotes(notebookId);
let get = id => Db.getNote(id);
let add = notes => Db.addNotes(notes);
let create = (notebookId: string) => Db.createNote(notebookId);
let update = (note: Data.note, ~sync=true, ()) =>
  Db.updateNote(note, ~sync, ());
let delete = (noteId: string, ~sync=true, ()) =>
  Db.deleteNote(noteId, ~sync, ());

DataSync.setNoteSyncedListener(note => Db.updateNote(note, ~sync=false, ()));
