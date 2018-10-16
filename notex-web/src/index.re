[%bs.raw {|require('trix')|}];
[%bs.raw {|require('trix/dist/trix.css')|}];

[@bs.module "./registerServiceWorker"]
external registerServiceWorker: unit => unit = "default";

Auth.check();
ReactDOMRe.renderToElementWithId(<App />, "root");

if (Utils.hot) {
  Utils.accept();
};

registerServiceWorker();

DbSync.run();
DataSyncRetry.getPendingChanges()->Future.get(DataSync.start);
