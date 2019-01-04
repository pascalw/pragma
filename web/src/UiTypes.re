module NoteCollection = {
  module CollectionKind = {
    type t =
      | Recents;

    let toString =
      fun
      | Recents => "Recents";
  };

  module Collection = {
    type t = {
      kind: CollectionKind.t,
      noteCount: int,
    };

    let id = t => CollectionKind.toString(t.kind);
    let name = id;
  };

  type t =
    | Notebook(Data.notebook)
    | Collection(Collection.t);

  let supportsNoteCreation =
    fun
    | Notebook(_) => true
    | Collection(_) => false;

  let makeCollection = (kind, noteCount): Collection.t => {
    {kind, noteCount};
  };

  let id =
    fun
    | Notebook(notebook) => notebook.id
    | Collection(collection) => Collection.id(collection);

  let fromNotebook = notebook => Notebook(notebook);
  let fromCollection = collection => Collection(collection);

  let dummy = () => {
    let collection: Collection.t = {kind: CollectionKind.Recents, noteCount: 10};

    Collection(collection);
  };
};
