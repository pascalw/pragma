open Belt;

[@bs.module] external styles: Js.Dict.t(string) = "./ListView.scss";
let style = name => Js.Dict.get(styles, name)->Option.getExn;

type listItem('a) = {
  id: string,
  title: string,
  count: option(int),
  model: 'a,
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

      <li
        key={item.id}
        className={isSelected ? style("selected") : ""}
        onClick={_e => onItemSelected(item)}>
        {
          switch (renderItemContent) {
          | None => defaultRenderItemContent(item)
          | Some(renderItemContent) => renderItemContent(item)
          }
        }
      </li>;
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
