open Belt;

[@bs.module] external styles: Js.Dict.t(string) = "./ListView.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

type listItem('a) = {
  id: string,
  title: string,
  count: option(int),
  model: 'a,
};

module ListItem = {
  type state = {pressTimer: ref(option(Js.Global.timeoutId))};

  type action;

  let component = ReasonReact.reducerComponent("ListItem");

  let make =
      (~selected: bool, ~onClick, ~onDoubleClick, ~onLongpress, children) => {
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
          onDoubleClick
          onTouchStart={self.handle(handlePress)}
          onMouseDown={self.handle(handlePress)}
          onTouchEnd={self.handle(handleRelease)}
          onMouseUp={self.handle(handleRelease)}>
          ...children
        </li>,
    };
  };
};

let defaultRenderItemContent = (item: listItem('a)) =>
  <div className={style("itemContentWrapper")}>
    <span className={style("title")}>
      {ReasonReact.string(item.title)}
    </span>
    {
      if (item.count->Option.isSome) {
        <span className={style("count")}>
          {item.count |> Option.getExn |> string_of_int |> ReasonReact.string}
        </span>;
      } else {
        ReasonReact.null;
      }
    }
  </div>;

let component = ReasonReact.statelessComponent("ListView");

let make =
    (
      ~items: list(listItem('a)),
      ~selectedId: option(string),
      ~onItemSelected: listItem('a) => unit,
      ~onItemDoubleClick=?,
      ~onItemLongpress=?,
      ~minWidth=?,
      ~renderItemContent=?,
      ~renderFooter=?,
      _children,
    ) => {
  ...component,
  render: _self => {
    let renderItem = item => {
      let isSelected =
        switch (selectedId) {
        | None => false
        | Some(selectedId) => selectedId == item.id
        };

      <ListItem
        key={item.id}
        selected=isSelected
        onLongpress={
          _e =>
            switch (onItemLongpress) {
            | None => ()
            | Some(fn) => fn(item) |> ignore
            }
        }
        onDoubleClick={
          _e =>
            switch (onItemDoubleClick) {
            | None => ()
            | Some(fn) => fn(item) |> ignore
            }
        }
        onClick={_e => onItemSelected(item)}>
        {
          switch (renderItemContent) {
          | None => defaultRenderItemContent(item)
          | Some(renderItemContent) => renderItemContent(item)
          }
        }
      </ListItem>;
    };

    let inlineStyle =
      if (Option.isSome(minWidth)) {
        ReactDOMRe.Style.make(~minWidth=minWidth->Option.getExn, ());
      } else {
        ReactDOMRe.Style.make();
      };

    <div className={style("listView")} style=inlineStyle>
      <ul>
        {List.map(items, renderItem) |> List.toArray |> ReasonReact.array}
      </ul>
      {
        switch (renderFooter) {
        | None => ReasonReact.null
        | Some(renderFun) =>
          <div className={style("footer")}> {renderFun()} </div>
        }
      }
    </div>;
  },
};
