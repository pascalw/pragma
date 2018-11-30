[@bs.module] external styles: Js.Dict.t(string) = "./SyncState.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let pendingSyncOperationsText =
  fun
  | "1" => "1 pending sync operation"
  | n => n ++ " pending sync operations";

let component = ReasonReact.statelessComponent("SyncState");
let make = (~pendingChangesCount, _children) => {
  ...component,
  render: _self => {
    let pendingChangesString = string_of_int(pendingChangesCount);
    let className =
      switch (pendingChangesCount) {
      | 0 => Utils.classnames([|style("syncState"), style("complete")|])
      | _ => style("syncState")
      };

    <div title={pendingSyncOperationsText(pendingChangesString)} className>
      <Icon icon=Icon.Sync />
      <span> {ReasonReact.string(pendingChangesString)} </span>
    </div>;
  },
};
