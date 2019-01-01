type toastData = {
  message: string,
  actionText: string,
  onAction: unit => unit,
};

let listener: ref(option(toastData => unit)) = ref(None);

let setListener = newListener => listener := Some(newListener);
let removeListener = () => listener := None;

let show = (message, actionText, onAction) =>
  switch (listener^) {
  | None => ()
  | Some(listener) => listener({message, actionText, onAction})
  };

[@bs.module] external styles: Js.Dict.t(string) = "./Toast.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

[@bs.module "../assets/close.svg"] external closeIcon: string = "default";

module ToastComponent = {
  let component = ReasonReact.statelessComponent("Toast");
  let make = (~message: string, ~onAction, ~onClose, _children) => {
    ...component,
    render: _self =>
      <div className={style("toast")}>
        <p> {ReasonReact.string(message)} </p>
        <button className={style("action")} onClick={_ => onAction()}>
          {ReasonReact.string("Update")}
        </button>
        <div className={style("close")} onClick={_ => onClose()}>
          <Icon icon=Icon.Close />
        </div>
      </div>,
  };
};

module Container = {
  type state = option(toastData);

  type action =
    | Show(toastData)
    | Close;

  let component = ReasonReact.reducerComponent("ToastContainer");
  let make = _children => {
    let performAction = (action, send, ()) => {
      action();
      send(Close);
    };

    {
      ...component,
      initialState: () => None,
      reducer: (action, _state) =>
        switch (action) {
        | Show(toastData) => ReasonReact.Update(Some(toastData))
        | Close => ReasonReact.Update(None)
        },
      didMount: self => {
        setListener(toastData => self.send(Show(toastData)));
        self.onUnmount(removeListener);
      },
      render: self =>
        switch (self.state) {
        | None => ReasonReact.null
        | Some((data: toastData)) =>
          <ToastComponent
            message={data.message}
            onAction={performAction(data.onAction, self.send)}
            onClose={() => self.send(Close)}
          />
        },
    };
  };
};
