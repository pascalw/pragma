[@bs.module] external styles: Js.Dict.t(string) = "./Onboarding.scss";
let style = name => Js.Dict.get(styles, name)->Belt.Option.getExn;

type onboardingPhase =
  | Start
  | Initializing
  | Done;

type state = {onboardingPhase};

type action =
  | Proceed(onboardingPhase);

let component = ReasonReact.reducerComponent("OnboardingContainer");
let make = (~onCompleted, _children) => {
  ...component,
  initialState: () => {onboardingPhase: Start},
  reducer: (action: action, _state: state) =>
    switch (action) {
    | Proceed(phase) => ReasonReact.Update({onboardingPhase: phase})
    },
  render: self =>
    <div className={style("container")}>
      <div className={style("inner")}>
        {switch (self.state.onboardingPhase) {
         | Start => <LoginContainer onLoggedIn=onCompleted />
         | _ => <div />
         }}
      </div>
    </div>,
};
