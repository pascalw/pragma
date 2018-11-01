let all = () => Db.getNotebooks();
let get = id => Db.getNotebook(id);
let add = notebooks => Db.addNotebooks(notebooks);

let create = notebook =>
  Db.createNotebook(notebook)->Future.tap(DataSync.pushNewNotebook);

let update = (notebook: Data.notebook, ~sync=true, ()) => {
  let now = Js.Date.fromFloat(Js.Date.now());
  Db.updateNotebook({...notebook, updatedAt: now}, ~sync, ());
};

let delete = (id, ~sync=true, ()) => Db.deleteNotebook(id, ~sync, ());

DataSync.setNotebookSyncedListener(notebook =>
  update(notebook, ~sync=false, ())
);
