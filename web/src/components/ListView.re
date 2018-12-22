open Belt;

[@bs.module] external styles: Js.Dict.t(string) = "./ListView.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

module Item = {
  type state = {pressTimer: ref(option(Js.Global.timeoutId))};

  type action;

  let component = ReasonReact.reducerComponent("ListItem");

  let make =
      (~selected: bool, ~onClick, ~onDoubleClick=?, ~onLongpress, children) => {
    let handlePress = (_e, self) => {
      switch (self.ReasonReact.state.pressTimer^) {
      | None => ()
      | Some(timerId) => Js.Global.clearTimeout(timerId)
      };

      self.ReasonReact.state.pressTimer :=
        Some(Js.Global.setTimeout(() => onLongpress(), 1000));
    };
    let handleRelease = (_e, self) =>
      switch (self.ReasonReact.state.pressTimer^) {
      | None => ()
      | Some(timerId) => Js.Global.clearTimeout(timerId)
      };

    {
      ...component,
      initialState: () => {pressTimer: ref(None)},
      reducer: (_action: action, _state: state) => ReasonReact.NoUpdate,
      render: self =>
        <li
          className={selected ? style("selected") : ""}
          onClick
          ?onDoubleClick
          onTouchStart={self.handle(handlePress)}
          onMouseDown={self.handle(handlePress)}
          onTouchEnd={self.handle(handleRelease)}
          onMouseUp={self.handle(handleRelease)}>
          ...children
        </li>,
    };
  };
};

module ItemContent = {
  let component = ReasonReact.statelessComponent("ListItemContent");

  let make = (~title, ~count, _children) => {
    ...component,
    render: _self =>
      <div className={style("itemContentWrapper")}>
        <span className={style("title")}> {ReasonReact.string(title)} </span>
        <span className={style("count")}>
          {count |> string_of_int |> ReasonReact.string}
        </span>
      </div>,
  };
};

module ItemContainer = {
  let component = ReasonReact.statelessComponent("ListItemContainer");

  let make = children => {
    ...component,
    render: _self => <ul> ...children </ul>,
  };
};

module Footer = {
  let component = ReasonReact.statelessComponent("ListFooter");

  let make = children => {
    ...component,
    render: _self => <div className={style("footer")}> ...children </div>,
  };
};

let className = hidden =>
  hidden ?
    Utils.classnames([|style("listView"), style("hidden")|]) :
    style("listView");

let component = ReasonReact.statelessComponent("ListView");
let make = (~minWidth=?, ~hidden=false, children) => {
  ...component,
  render: _self => {
    let inlineStyle =
      if (Option.isSome(minWidth)) {
        ReactDOMRe.Style.make(~minWidth=minWidth->Option.getExn, ());
      } else {
        ReactDOMRe.Style.make();
      };

    <div className={className(hidden)} style=inlineStyle> ...children </div>;
  },
};
