[@bs.module] external styles: Js.Dict.t(string) = "./Link.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

let openInNewTab = url =>
  Webapi.Dom.Window.open_(
    ~url,
    ~name="_blank",
    ~features="",
    Webapi.Dom.window,
  )
  |> ignore;

module LinkPanel = {
  let openInNewTab = (url, closePanel, e) => {
    ReactEvent.Mouse.preventDefault(e);

    openInNewTab(url);
    closePanel();
  };

  type state = {anchorRef: ref(option(Webapi.Dom.Element.t))};

  type action =
    | SetAnchorRef(ReasonReact.reactRef);

  let setAnchorRef = (anchorRef, {ReasonReact.state}) =>
    state.anchorRef := Js.Nullable.toOption(anchorRef);

  let component = ReasonReact.reducerComponent("LinkPanel");
  let make = (~url, ~onClose, _children) => {
    ...component,
    initialState: () => {anchorRef: ref(None)},
    reducer: (_action: action, _state: state) => ReasonReact.NoUpdate,
    didMount: self => {
      let onDocumentClick = (e: Dom.event) =>
        switch (self.state.anchorRef^) {
        | Some(anchor) =>
          let target = EventRe.target(e);
          if (!
                ElementRe.contains(
                  EventTargetRe.unsafeAsElement(target),
                  anchor,
                )) {
            onClose();
          };
        | _ => ()
        };

      Webapi.Dom.Document.addEventListenerUseCapture(
        "click",
        onDocumentClick,
        Webapi.Dom.document,
      );

      self.onUnmount(() =>
        Webapi.Dom.Document.removeEventListenerUseCapture(
          "click",
          onDocumentClick,
          Webapi.Dom.document,
        )
      );
    },
    render: self =>
      <div className={style("panel")} contentEditable=false readOnly=true>
        <a
          href=url
          onClick={openInNewTab(url, onClose)}
          ref={self.handle(setAnchorRef)}
          contentEditable=false
          readOnly=true>
          {ReasonReact.string(url)}
        </a>
      </div>,
  };
};

type state = {panelVisible: bool};

type action =
  | TogglePanel(bool);

let component = ReasonReact.reducerComponent("Link");

let make = (~url, children) => {
  ...component,
  initialState: () => {panelVisible: false},
  reducer: (action: action, _state: state) =>
    switch (action) {
    | TogglePanel(visible) => ReasonReact.Update({panelVisible: visible})
    },
  render: self =>
    <div>
      <a
        href=url
        onClick={_ => self.send(TogglePanel(!self.state.panelVisible))}>
        ...children
      </a>
      {
        self.state.panelVisible ?
          <LinkPanel url onClose={() => self.send(TogglePanel(false))} /> :
          ReasonReact.null
      }
    </div>,
};

[@bs.deriving abstract]
type jsProps = {
  url: string,
  children: array(ReasonReact.reactElement),
};

let jsComponent =
  ReasonReact.wrapReasonForJs(~component, jsProps =>
    make(~url=jsProps->urlGet, jsProps->childrenGet)
  );
