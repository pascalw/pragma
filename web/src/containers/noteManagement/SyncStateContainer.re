type state = {pendingChanges: int};

type action =
  | SetPendingChanges(int);

let component = ReasonReact.reducerComponent("SyncStateContainer");
let make = _children => {
  ...component,
  initialState: () => {pendingChanges: 0},
  reducer: (action: action, _state: state) =>
    switch (action) {
    | SetPendingChanges(number) => ReasonReact.Update({pendingChanges: number})
    },
  didMount: self => {
    DataSync.setPendingChangesListener(pendingChangesCount =>
      self.send(SetPendingChanges(pendingChangesCount))
    );

    self.onUnmount(() => DataSync.removePendingChangesListener());
  },
  render: self => <SyncState pendingChangesCount={self.state.pendingChanges} />,
};
