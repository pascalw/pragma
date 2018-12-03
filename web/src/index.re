[%bs.raw {|require('draft-js/dist/Draft.css')|}];

[@bs.module "./registerServiceWorker"]
external registerServiceWorker: (unit => unit) => unit = "default";

ReactDOMRe.renderToElementWithId(<App />, "root");

if (Utils.hot) {
  Utils.accept();
};

registerServiceWorker(() =>
  Toast.show("New version available.", "Update", () =>
    LocationRe.reload(Webapi.Dom.location)
  )
);
