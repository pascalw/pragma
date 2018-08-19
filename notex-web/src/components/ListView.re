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

let component = ReasonReact.statelessComponent("NotebooksListing");

let make =
    (
      ~items: list(listItem('a)),
      ~selectedId: option(string),
      ~onItemSelected: listItem('a) => unit,
      ~renderItemContent=?,
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

    <div className={style("listView")}>
      <ul>
        {List.map(items, renderItem) |> List.toArray |> ReasonReact.array}
      </ul>
    </div>;
  },
};
