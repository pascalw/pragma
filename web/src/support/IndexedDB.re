type value = Js.Json.t;

type keyPath = string;
type validKey = string; /* IndexedDB supports other types of keys, but we don't need it */
type dbName = string;
type storeName = string;

module Index = {
  type t;

  [@bs.send]
  external getAllByKey: (t, validKey) => Js.Promise.t(Js.Array.t(value)) =
    "getAll";
};

module ObjectStoreParams = {
  [@bs.deriving abstract]
  type t = {keyPath: option(keyPath)};

  let make = (~keyPath: keyPath): t => t(~keyPath=Some(keyPath));
};

module CreateIndexParams = {
  [@bs.deriving abstract]
  type t = {
    unique: bool,
    multiEntry: bool,
  };

  let make = (~unique: bool, ~multiEntry: bool): t =>
    t(~unique, ~multiEntry);
};

module ObjectStore = {
  type t;

  [@bs.send] external add: (t, value) => Js.Promise.t(unit) = "";
  [@bs.send] external put: (t, value) => Js.Promise.t(unit) = "";
  [@bs.send] external delete: (t, validKey) => Js.Promise.t(unit) = "";
  [@bs.send] external clear: t => Js.Promise.t(unit) = "";

  [@bs.send] external get: (t, validKey) => Js.Promise.t(Js.Nullable.t(value)) = "";
  let get = (store, key) => {
    get(store, key)
    |> Js.Promise.then_(value => Js.Promise.resolve(Js.Nullable.toOption(value)))
  };

  [@bs.send]
  external getAll: t => Js.Promise.t(Js.Array.t(value)) = "";

  type indexName = string;
  [@bs.send]
  external createIndex: (t, indexName, keyPath, CreateIndexParams.t) => Index.t =
    "";
  [@bs.send] external index: (t, indexName) => Index.t = "";
};

module Transaction = {
  type t;

  [@bs.send] external objectStore: (t, storeName) => ObjectStore.t = "";
  [@bs.get] external complete: t => Js.Promise.t(unit) = "";

  type mode =
    | ReadOnly
    | ReadWrite;

  let encodeMode =
    fun
    | ReadOnly => "readonly"
    | ReadWrite => "readwrite";
};

module UpgradeDb = {
  type t;

  [@bs.send]
  external createObjectStore:
    (t, storeName, ObjectStoreParams.t) => ObjectStore.t =
    "";
  [@bs.send] external deleteObjectStore: (t, storeName) => unit = "";

  [@bs.get] external version: t => int = "";
  [@bs.get] external oldVersion: t => int = "";
  [@bs.get] external transaction: t => Transaction.t = "";
};

module DB = {
  type t;

  [@bs.send] external close: t => unit = "";
  [@bs.send]
  external transaction: (t, storeName, string) => Transaction.t = "";
  let transaction = (db, storeName, mode: Transaction.mode) =>
    transaction(db, storeName, Transaction.encodeMode(mode));
};

[@bs.module "idb"]
external open_: (dbName, int, UpgradeDb.t => unit) => Js.Promise.t(DB.t) =
  "open";
[@bs.module "idb"] external delete: string => Js.Promise.t(unit) = "";
