open Belt;

type tag = string;
type language = string;

type content =
  | TextContent(string)
  | CodeContent(string, language);

type note = {
  id: int,
  title: string,
  tags: list(tag),
  contentBlocks: list(content),
  createdAt: Js.Date.t,
  updatedAt: Js.Date.t,
};

type notebook = {
  id: int,
  name: string,
  notes: list(note),
  createdAt: Js.Date.t,
};

type state = {
  notebooks: option(list(notebook)),
  selectedNotebook: option(notebook),
  editingNote: option(note),
};

type action =
  | SelectNotebook(notebook)
  | SelectNote(note)
  | UpdateNoteText(notebook, note, content, string);

let reducer = (action: action, state: state) =>
  switch (action) {
  | SelectNotebook(notebook) =>
    let editingNote = notebook.notes |> List.head;

    {...state, selectedNotebook: Some(notebook), editingNote}
    |. ReasonReact.Update;

  | SelectNote(note) =>
    {...state, editingNote: Some(note)} |. ReasonReact.Update

  | UpdateNoteText(notebook, note, content, text) =>
    let updatedContentBlocks =
      List.map(note.contentBlocks, contentBlock =>
        if (contentBlock == content) {
          TextContent(text);
        } else {
          contentBlock;
        }
      );

    let updatedNote = {...note, contentBlocks: updatedContentBlocks};

    let updatedNotes =
      List.map(notebook.notes, n =>
        if (n == note) {
          updatedNote;
        } else {
          n;
        }
      );

    let updatedNotebook = {...notebook, notes: updatedNotes};

    let updatedNotebooks =
      Option.map(state.notebooks, notebooks =>
        List.map(notebooks, nb =>
          if (nb == notebook) {
            updatedNotebook;
          } else {
            nb;
          }
        )
      );

    {
      notebooks: updatedNotebooks,
      editingNote: Some(updatedNote),
      selectedNotebook: Some(updatedNotebook),
    }
    |. ReasonReact.Update;
  };

module NotebooksListing = {
  let component = ReasonReact.statelessComponent("NotebooksListing");

  let make =
      (
        ~notebooks: option(list(notebook)),
        ~selectedNotebook: option(notebook),
        ~send,
        _children,
      ) => {
    ...component,
    render: _self => {
      let renderNotebook = notebook => {
        let isSelected =
          switch (selectedNotebook) {
          | None => false
          | Some(selectedNotebook) => selectedNotebook.id == notebook.id
          };

        <li
          key=(notebook.id |> string_of_int)
          className=(isSelected ? "selected" : "")
          onClick=(_e => send(SelectNotebook(notebook)))>
          <span className="notebooks__list_name">
            (ReasonReact.string(notebook.name))
          </span>
          <span className="notebooks__list__note-count">
            (
              notebook.notes
              |> List.length
              |> string_of_int
              |> ReasonReact.string
            )
          </span>
        </li>;
      };

      <div className="notebooks">
        (
          switch (notebooks) {
          | None => <p> (ReasonReact.string("There are no notebooks.")) </p>
          | Some(notebooks) =>
            <ul>
              (
                notebooks
                |. List.map(renderNotebook)
                |. List.toArray
                |. ReasonReact.array
              )
            </ul>
          }
        )
      </div>;
    },
  };
};

module NotesListing = {
  let component = ReasonReact.statelessComponent("NotesListing");

  let formatDate = date => DateFns.format("D MMMM YYYY", date);

  let make =
      (
        ~notebook: option(notebook),
        ~selectedNote: option(note),
        ~send,
        _children,
      ) => {
    ...component,
    render: _self => {
      let renderNote = (note: note) => {
        let isSelected =
          switch (selectedNote) {
          | None => false
          | Some(selectedNote) => selectedNote.id == note.id
          };

        <li
          key=(note.id |> string_of_int)
          className=(isSelected ? "selected" : "")
          onClick=(_e => send(SelectNote(note)))>
          (ReasonReact.string(note.title))
          <br />
          <small> (ReasonReact.string(note.updatedAt |> formatDate)) </small>
        </li>;
      };

      <div className="notes">
        (
          switch (notebook) {
          | None => <p> (ReasonReact.string("There are no notes.")) </p>
          | Some(notebook) =>
            <ul>
              (
                notebook.notes
                |. List.map(renderNote)
                |. List.toArray
                |. ReasonReact.array
              )
            </ul>
          }
        )
      </div>;
    },
  };
};

module NoteEditor = {
  let component = ReasonReact.statelessComponent("NoteEditor");

  let renderContentBlocks = (notebook, note: note, send) =>
    switch (note.contentBlocks |> List.head) {
    | Some(TextContent(text) as content) =>
      <TrixEditor
        key=(note.id |> string_of_int)
        text
        onChange=(
          value => send(UpdateNoteText(notebook, note, content, value))
        )
      />
    | _ => <p> (ReasonReact.string("FIXME: unsupported content type.")) </p>
    };

  let make = (~notebook, ~note, ~send, _children) => {
    ...component,
    render: _self =>
      switch (note) {
      | None =>
        <div className="editor">
          <p> (ReasonReact.string("No note selected")) </p>
        </div>
      | Some(note) =>
        <div className="editor">
          <h2 className="note-title"> (note.title |. ReasonReact.string) </h2>
          <div className="content">
            (renderContentBlocks(notebook, note, send))
          </div>
        </div>
      },
  };
};

let now = Js.Date.make();

let buildNote =
    (id: int, title: string, createdAt: Js.Date.t, content: string)
    : note => {
  let content = TextContent(content);
  {
    id,
    title,
    tags: [],
    contentBlocks: [content],
    createdAt,
    updatedAt: createdAt,
  };
};

let notebooks = [
  {
    id: 1,
    name: "Pascal",
    notes: [
      buildNote(1, "Note 1 by Pascal", now, "Hello, world! This is note 1."),
      buildNote(2, "Note 2 by Pascal", now, "Hello, world! This is note 2."),
    ],
    createdAt: now,
  },
  {
    id: 2,
    name: "Testing",
    notes: [
      buildNote(3, "Note 1 for Testing", now, "Hello, just testing note 1!"),
      buildNote(4, "Note 2 for Testing", now, "Hello, just testing note 2!"),
    ],
    createdAt: now,
  },
];

let component = ReasonReact.reducerComponent("App");

let make = _children => {
  ...component,
  initialState: () => {
    notebooks: Some(notebooks),
    selectedNotebook: notebooks |> List.head,
    editingNote:
      notebooks
      |> List.head
      |> Js.Option.andThen((. n) => n.notes |> List.head),
  },
  reducer,
  render: self =>
    <main>
      <div className="columns">
        <NotebooksListing
          notebooks=self.state.notebooks
          selectedNotebook=self.state.selectedNotebook
          send=self.send
        />
        <NotesListing
          notebook=self.state.selectedNotebook
          selectedNote=self.state.editingNote
          send=self.send
        />
        <NoteEditor
          notebook=(self.state.selectedNotebook |> Option.getExn)
          note=self.state.editingNote
          send=self.send
        />
      </div>
    </main>,
};
