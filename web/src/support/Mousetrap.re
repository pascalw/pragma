type mouseEvent = Webapi.Dom.MouseEvent.t;

[@bs.module "mousetrap"] external bind: (string, mouseEvent => unit) => unit = "";
[@bs.module "mousetrap"] external unbind: string => unit = "";
