[%bs.raw {|require('codemirror/lib/codemirror.css')|}];

type editor;
[@bs.send] external setOption: (editor, string, string) => unit = "";

type cmMode = {
  name: string,
  install: unit => Js.Promise.t(unit),
};

module SupportedLanguageMap = Belt.Map.String;

let supportedLanguages: SupportedLanguageMap.t(cmMode) =
  SupportedLanguageMap.(
    empty
    ->set(
        "go",
        {
          name: "Go",
          install: () => Utils.import("codemirror/mode/ruby/ruby"),
        },
      )
    ->set(
        "python",
        {
          name: "Python",
          install: () => Utils.import("codemirror/mode/python/python"),
        },
      )
    ->set(
        "css",
        {
          name: "CSS",
          install: () => Utils.import("codemirror/mode/css/css"),
        },
      )
    ->set(
        "clojure",
        {
          name: "Clojure",
          install: () => Utils.import("codemirror/mode/clojure/clojure"),
        },
      )
    ->set(
        "gherkin",
        {
          name: "Gherkin",
          install: () => Utils.import("codemirror/mode/gherkin/gherkin"),
        },
      )
    ->set(
        "dart",
        {
          name: "Dart",
          install: () => Utils.import("codemirror/mode/dart/dart"),
        },
      )
    ->set(
        "shell",
        {
          name: "Shell",
          install: () => Utils.import("codemirror/mode/shell/shell"),
        },
      )
    ->set(
        "markdown",
        {
          name: "Markdown",
          install: () => Utils.import("codemirror/mode/markdown/markdown"),
        },
      )
    ->set(
        "perl",
        {
          name: "Perl",
          install: () => Utils.import("codemirror/mode/perl/perl"),
        },
      )
    ->set(
        "rust",
        {
          name: "Rust",
          install: () => Utils.import("codemirror/mode/rust/rust"),
        },
      )
    ->set(
        "rst",
        {
          name: "reStructuredText",
          install: () => Utils.import("codemirror/mode/rst/rst"),
        },
      )
    ->set(
        "erlang",
        {
          name: "Erlang",
          install: () => Utils.import("codemirror/mode/erlang/erlang"),
        },
      )
    ->set(
        "elm",
        {
          name: "Elm",
          install: () => Utils.import("codemirror/mode/elm/elm"),
        },
      )
    ->set(
        "crystal",
        {
          name: "Crystal",
          install: () => Utils.import("codemirror/mode/crystal/crystal"),
        },
      )
    ->set(
        "html",
        {
          name: "HTML",
          install: () => Utils.import("codemirror/mode/htmlmixed/htmlmixed"),
        },
      )
    ->set(
        "haskell",
        {
          name: "Haskell",
          install: () => Utils.import("codemirror/mode/haskell/haskell"),
        },
      )
    ->set(
        "php",
        {
          name: "PHP",
          install: () => Utils.import("codemirror/mode/php/php"),
        },
      )
    ->set(
        "lua",
        {
          name: "Lua",
          install: () => Utils.import("codemirror/mode/lua/lua"),
        },
      )
    ->set(
        "xml",
        {
          name: "XML",
          install: () => Utils.import("codemirror/mode/xml/xml"),
        },
      )
    ->set(
        "powershell",
        {
          name: "Powershell",
          install: () =>
            Utils.import("codemirror/mode/powershell/powershell"),
        },
      )
    ->set(
        "swift",
        {
          name: "Swift",
          install: () => Utils.import("codemirror/mode/swift/swift"),
        },
      )
    ->set(
        "yaml",
        {
          name: "YAML",
          install: () => Utils.import("codemirror/mode/yaml/yaml"),
        },
      )
    ->set(
        "groovy",
        {
          name: "Groovy",
          install: () => Utils.import("codemirror/mode/groovy/groovy"),
        },
      )
    ->set(
        "java",
        {
          name: "Java",
          install: () => Utils.import("codemirror/mode/clike/clike"),
        },
      )
    ->set(
        "javascript",
        {
          name: "JavaScript",
          install: () =>
            Utils.import("codemirror/mode/javascript/javascript"),
        },
      )
    ->set(
        "sass",
        {
          name: "Sass",
          install: () => Utils.import("codemirror/mode/sass/sass"),
        },
      )
    ->set(
        "ruby",
        {
          name: "Ruby",
          install: () => Utils.import("codemirror/mode/ruby/ruby"),
        },
      )
    ->set(
        "sql",
        {
          name: "SQL",
          install: () => Utils.import("codemirror/mode/sql/sql"),
        },
      )
  );

[@bs.module "react-codemirror2"]
external codeMirrorReact: ReasonReact.reactClass = "Controlled";

let code = (block: Data.contentBlock) =>
  switch (block.content) {
  | Data.CodeContent(code, _language) => code
  | _ => Js.Exn.raiseError("Unsupported content block")
  };

let language = (block: Data.contentBlock) =>
  switch (block.content) {
  | Data.CodeContent(_code, language) when language != "" => language
  | _ => "unknown"
  };

let mapMode = language =>
  switch (language) {
  | "sql" => "text/x-sql"
  | "php" => "text/x-php"
  | "java" => "text/x-java"
  | "html" => "text/html"
  | _ => language
  };

let loadMode = (contentBlock, editor) => {
  let language = language(contentBlock);
  let mode = SupportedLanguageMap.getExn(supportedLanguages, language);

  mode.install()
  ->FutureJs.fromPromise(Js.String.make)
  ->Future.get(_ => setOption(editor, "mode", mapMode(language)));
};

module CodeMirror = {
  [@bs.deriving abstract]
  type cmOptions = {
    mode: string,
    theme: string,
    lineNumbers: bool,
    lineWrapping: bool,
  };

  [@bs.deriving abstract]
  type jsProps = {
    value: string,
    options: cmOptions,
    onBeforeChange: (string, string, string) => unit,
    editorDidMount: editor => unit,
  };

  let options =
    cmOptions(~theme="default", ~lineNumbers=true, ~lineWrapping=true);

  let make =
      (
        ~mode: string,
        ~code: string,
        ~editorDidMount,
        ~onBeforeChange,
        children,
      ) =>
    ReasonReact.wrapJsForReason(
      ~reactClass=codeMirrorReact,
      ~props=
        jsProps(
          ~value=code,
          ~options=options(~mode),
          ~onBeforeChange,
          ~editorDidMount,
        ),
      children,
    );
};

module CodeMirrorWrapper = {
  type state = {editor: option(editor)};

  type action =
    | SetEditor(editor);

  let component = ReasonReact.reducerComponent("CodeMirrorWrapper");
  let make = (~contentBlock: Data.contentBlock, ~onChange, _children) => {
    ...component,
    initialState: () => {editor: None},
    reducer: (action: action, _state: state) =>
      switch (action) {
      | SetEditor(editor) => ReasonReact.Update({editor: Some(editor)})
      },
    didUpdate: oldAndNewSelf =>
      switch (oldAndNewSelf.newSelf.state.editor) {
      | None => ()
      | Some(editor) => loadMode(contentBlock, editor)
      },
    render: self => {
      let code = code(contentBlock);
      let language = language(contentBlock);

      <CodeMirror
        mode={mapMode(language)}
        code
        onBeforeChange=onChange
        editorDidMount={editor => self.send(SetEditor(editor))}
      />;
    },
  };
};

let typeOptions = () =>
  <optgroup label="Code">
    {
      SupportedLanguageMap.toList(supportedLanguages)
      ->Belt.List.map(((id, mode)) =>
          <option key=id value=id> {ReasonReact.string(mode.name)} </option>
        )
      ->Belt.List.toArray
      ->ReasonReact.array
    }
  </optgroup>;

let component = ReasonReact.statelessComponent("CodeEditor");
let make = (~contentBlock: Data.contentBlock, ~onChange, _children) => {
  let onChange = (_, _, value) => onChange(value);

  {
    ...component,
    render: _self => <CodeMirrorWrapper contentBlock onChange />,
  };
};
