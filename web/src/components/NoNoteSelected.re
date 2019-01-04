[@bs.module] external styles: Js.Dict.t(string) = "./NoNoteSelected.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let component = ReasonReact.statelessComponent("NoNoteSelected");

let make = _children => {
  ...component,
  render: _self =>
    <div className={style("container")}> {ReasonReact.string("No note selected")} </div>,
};
