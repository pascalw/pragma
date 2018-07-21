[%bs.raw {|require('./index.scss')|}];

[%bs.raw {|require('trix')|}];
[%bs.raw {|require('trix/dist/trix.css')|}];

[@bs.module "./registerServiceWorker"]
external register_service_worker : unit => unit = "default";

ReactDOMRe.renderToElementWithId(<App />, "root");

if (Utils.hot) {
  Utils.accept();
};

register_service_worker();
