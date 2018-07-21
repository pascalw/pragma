open Belt;

[@bs.module] external styles : Js.Dict.t(string) = "./index.scss";
let style = name => Js.Dict.get(styles, name) |. Option.getExn;

open Data;
open State;

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
        <div className=(style("editor"))>
          <p> (ReasonReact.string("No note selected")) </p>
        </div>
      | Some(note) =>
        <div className=(style("editor"))>
          <h2 className=(style("note-title"))>
            (note.title |. ReasonReact.string)
          </h2>
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

let renderNotebooks =
    (
      notebooks: option(list(notebook)),
      selectedNotebook: option(notebook),
      send,
    ) => {
  let listItems =
    switch (notebooks) {
    | None => []
    | Some(notebooks) =>
      notebooks
      |. List.map(notebook =>
           (
             {
               id: notebook.id |> string_of_int,
               title: notebook.name,
               count: Some(notebook.notes |> List.length),
               model: notebook,
             }:
               ListView.listItem(notebook)
           )
         )
    };

  <ListView
    items=listItems
    selectedId=(selectedNotebook |. Option.map(n => n.id |. string_of_int))
    onItemSelected=(item => send(SelectNotebook(item.model)))
  />;
};

let renderNotes =
    (selectedNotebook: option(notebook), editingNote: option(note), send) => {
  let listItems =
    switch (selectedNotebook) {
    | None => []
    | Some(selectedNotebook) =>
      selectedNotebook.notes
      |. List.map(note =>
           (
             {
               id: note.id |> string_of_int,
               title: note.title,
               count: None,
               model: note,
             }:
               ListView.listItem(note)
           )
         )
    };

  let formatDate = date => DateFns.format("D MMMM YYYY", date);
  let renderNoteListItemContent = (item: ListView.listItem(note)) =>
    <p>
      (ReasonReact.string(item.model.title))
      <br />
      <small>
        (ReasonReact.string(item.model.updatedAt |> formatDate))
      </small>
    </p>;

  <ListView
    items=listItems
    selectedId=(editingNote |. Option.map(n => n.id |. string_of_int))
    onItemSelected=(item => send(SelectNote(item.model)))
    renderItemContent=renderNoteListItemContent
  />;
};

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
    <main className=(style("main"))>
      <div className=(style("columns"))>
        (
          renderNotebooks(
            self.state.notebooks,
            self.state.selectedNotebook,
            self.send,
          )
        )
        (
          renderNotes(
            self.state.selectedNotebook,
            self.state.editingNote,
            self.send,
          )
        )
        <NoteEditor
          notebook=(self.state.selectedNotebook |> Option.getExn)
          note=self.state.editingNote
          send=self.send
        />
      </div>
    </main>,
};
