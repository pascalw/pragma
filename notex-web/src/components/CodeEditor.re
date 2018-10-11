[%bs.raw {|require('codemirror/lib/codemirror.css')|}];

/* TODO: lazy loading */

[%bs.raw {|require('codemirror/mode/go/go')|}];
[%bs.raw {|require('codemirror/mode/python/python')|}];
[%bs.raw {|require('codemirror/mode/css/css')|}];
[%bs.raw {|require('codemirror/mode/clojure/clojure')|}];
[%bs.raw {|require('codemirror/mode/gherkin/gherkin')|}];
[%bs.raw {|require('codemirror/mode/dart/dart')|}];
[%bs.raw {|require('codemirror/mode/shell/shell')|}];
[%bs.raw {|require('codemirror/mode/markdown/markdown')|}];
[%bs.raw {|require('codemirror/mode/perl/perl')|}];
[%bs.raw {|require('codemirror/mode/rust/rust')|}];
[%bs.raw {|require('codemirror/mode/rst/rst')|}];
[%bs.raw {|require('codemirror/mode/erlang/erlang')|}];
[%bs.raw {|require('codemirror/mode/elm/elm')|}];
[%bs.raw {|require('codemirror/mode/crystal/crystal')|}];
[%bs.raw {|require('codemirror/mode/htmlmixed/htmlmixed')|}];
[%bs.raw {|require('codemirror/mode/haskell/haskell')|}];
[%bs.raw {|require('codemirror/mode/php/php')|}];
[%bs.raw {|require('codemirror/mode/lua/lua')|}];
[%bs.raw {|require('codemirror/mode/xml/xml')|}];
[%bs.raw {|require('codemirror/mode/powershell/powershell')|}];
[%bs.raw {|require('codemirror/mode/swift/swift')|}];
[%bs.raw {|require('codemirror/mode/yaml/yaml')|}];
[%bs.raw {|require('codemirror/mode/groovy/groovy')|}];
[%bs.raw {|require('codemirror/mode/javascript/javascript')|}];
[%bs.raw {|require('codemirror/mode/sass/sass')|}];
[%bs.raw {|require('codemirror/mode/jsx/jsx')|}];
[%bs.raw {|require('codemirror/mode/ruby/ruby')|}];
[%bs.raw {|require('codemirror/mode/sql/sql')|}];

[@bs.module "react-codemirror2"]
external codeMirrorReact: ReasonReact.reactClass = "UnControlled";

let mapMode = language =>
  switch (language) {
  | "sql" => "text/x-sql"
  | "php" => "text/x-php"
  | _ => language
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
    onChange: (string, string, string) => unit,
  };

  let options =
    cmOptions(~theme="default", ~lineNumbers=true, ~lineWrapping=true);

  let make = (~contentBlock: Data.contentBlock, ~onChange, children) =>
    switch (contentBlock.content) {
    | Data.CodeContent(code, language) =>
      ReasonReact.wrapJsForReason(
        ~reactClass=codeMirrorReact,
        ~props=
          jsProps(
            ~value=code,
            ~options=options(~mode=mapMode(language)),
            ~onChange,
          ),
        children,
      )
    | _ => Js.Exn.raiseError("Unsupported content type")
    };
};

let component = ReasonReact.statelessComponent("CodeEditor");

let make = (~contentBlock: Data.contentBlock, ~onChange, _children) => {
  let onChange = (_, _, value) => onChange(value);

  {...component, render: _self => <CodeMirror contentBlock onChange />};
};
