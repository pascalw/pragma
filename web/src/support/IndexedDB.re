type value = Js.Json.t;

type keyPath = string;
type validKey = string; /* IndexedDB supports other types of keys, but we don't need it */
type dbName = string;
type storeName = string;

module Cursor = {
  type t;

  type direction =
    | Next
    | NextUnique
    | Prev
    | PrevUnique;

  let directionString =
    fun
    | Next => "next"
    | NextUnique => "nextunique"
    | Prev => "prev"
    | PrevUnique => "prevunique";

  [@bs.get] external value: t => value = "";

  [@bs.send] external continue: t => Js.Promise.t(Js.Nullable.t(t)) = "";
  let continue = cursor =>
    continue(cursor)
    |> Js.Promise.then_(nCursor => Js.Promise.resolve(Js.Nullable.toOption(nCursor)));

  let rec take = (cursor, count, result): Js.Promise.t(array(value)) => {
    let value = value(cursor);
    let result = Belt.Array.concat(result, [|value|]);

    if (Belt.Array.size(result) == count) {
      Js.Promise.resolve(result);
    } else {
      continue(cursor)
      |> Js.Promise.then_(
           fun
           | None => Js.Promise.resolve(result)
           | Some(cursor) => take(cursor, count, result),
         );
    };
  };
};

module Index = {
  type t;

  [@bs.send] external getAllByKey: (t, validKey) => Js.Promise.t(Js.Array.t(value)) = "getAll";

  [@bs.send] external countByKey_: (t, option(validKey)) => Js.Promise.t(int) = "count";
  let countByKey = (index, key) => countByKey_(index, Some(key));
  let count = index => countByKey_(index, None);

  [@bs.send] external openCursor: (t, option(validKey), string) => Js.Promise.t(Cursor.t) = "";
  let openCursor = (index, direction) =>
    openCursor(index, None, Cursor.directionString(direction));
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

  let make = (~unique: bool, ~multiEntry: bool): t => t(~unique, ~multiEntry);
};

module ObjectStore = {
  type t;

  [@bs.send] external add: (t, value) => Js.Promise.t(unit) = "";
  [@bs.send] external put: (t, value) => Js.Promise.t(unit) = "";
  [@bs.send] external delete: (t, validKey) => Js.Promise.t(unit) = "";
  [@bs.send] external clear: t => Js.Promise.t(unit) = "";

  [@bs.send] external get: (t, validKey) => Js.Promise.t(Js.Nullable.t(value)) = "";
  let get = (store, key) =>
    get(store, key)
    |> Js.Promise.then_(value => Js.Promise.resolve(Js.Nullable.toOption(value)));

  [@bs.send] external getAll: t => Js.Promise.t(Js.Array.t(value)) = "";

  type indexName = string;
  [@bs.send] external createIndex: (t, indexName, keyPath, CreateIndexParams.t) => Index.t = "";
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

  [@bs.send] external createObjectStore: (t, storeName, ObjectStoreParams.t) => ObjectStore.t = "";
  [@bs.send] external deleteObjectStore: (t, storeName) => unit = "";

  [@bs.get] external version: t => int = "";
  [@bs.get] external oldVersion: t => int = "";
  [@bs.get] external transaction: t => Transaction.t = "";
};

module DB = {
  type t;

  [@bs.send] external close: t => unit = "";
  [@bs.send] external transaction: (t, storeName, string) => Transaction.t = "";
  let transaction = (db, storeName, mode: Transaction.mode) =>
    transaction(db, storeName, Transaction.encodeMode(mode));
};

[@bs.module "idb"]
external open_: (dbName, int, UpgradeDb.t => unit) => Js.Promise.t(DB.t) = "open";
[@bs.module "idb"] external delete: string => Js.Promise.t(unit) = "";
