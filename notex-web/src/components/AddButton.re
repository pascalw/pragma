[@bs.module] external styles: Js.Dict.t(string) = "./AddButton.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let component = ReasonReact.statelessComponent("ListView");

let make = (~onClick, _children) => {
  ...component,
  render: _self => <button className={style("button")} onClick />,
};
