[@bs.module] external styles: Js.Dict.t(string) = "./AddButton.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let component = ReasonReact.statelessComponent("AddButton");

let make = (~onClick, _children) => {
  ...component,
  render: _self =>
    <button className={style("button")} onClick tabIndex=(-1)>
      <Icon icon=Icon.Add />
    </button>,
};
