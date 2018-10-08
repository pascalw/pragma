let component = ReasonReact.statelessComponent("NotebookTitleEditor");

let make = (~value, ~onComplete, _children) => {
  let select = [%raw {| function(input) { input.select() } |}];

  let onFocus = event => {
    let target = ReactEvent.Focus.target(event);
    select(. target);
  };

  let onBlur = event => {
    let target = ReactEvent.Focus.target(event);
    onComplete(target##value);
  };

  let onKeyPress = event => {
    let key = ReactEvent.Keyboard.key(event);
    let target = ReactEvent.Keyboard.target(event);

    if (key == "Enter") {
      onComplete(target##value);
    };
  };

  {
    ...component,
    render: _self =>
      <input
        type_="text"
        defaultValue=value
        autoFocus=true
        onFocus
        onKeyPress
        onBlur
        style={ReactDOMRe.Style.make(~fontSize=".9em", ~border="none", ())}
      />,
  };
};
