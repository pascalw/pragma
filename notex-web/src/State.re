open Data;

type state = {
  notebooks: option(list(notebook)),
  selectedNotebook: option(notebook),
  editingNote: option(note),
};

type action =
  | SelectNotebook(notebook)
  | SelectNote(note)
  | UpdateNoteText(notebook, note, content, string);
