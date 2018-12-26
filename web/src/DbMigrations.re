open IndexedDB;

module MigrationsMap = Belt.Map.Int;
type migration = IndexedDB.UpgradeDb.t => unit;

let notebooksStore = "notebooks";
let notesStore = "notes";
let contentBlocksStore = "contentBlocks";

let migrations: MigrationsMap.t(migration) =
  MigrationsMap.(
    empty
    ->set(
        0,
        upgradeDb => {
          UpgradeDb.createObjectStore(
            upgradeDb,
            notebooksStore,
            ObjectStoreParams.make(~keyPath="id"),
          )
          |> ignore;

          UpgradeDb.createObjectStore(
            upgradeDb,
            notesStore,
            ObjectStoreParams.make(~keyPath="id"),
          )
          |> ignore;

          let tx = UpgradeDb.transaction(upgradeDb);

          Transaction.objectStore(tx, notesStore)
          ->ObjectStore.createIndex(
              "forNotebook",
              "notebookId",
              CreateIndexParams.make(~unique=false, ~multiEntry=false),
            )
          |> ignore;

          UpgradeDb.createObjectStore(
            upgradeDb,
            contentBlocksStore,
            ObjectStoreParams.make(~keyPath="id"),
          )
          |> ignore;

          Transaction.objectStore(tx, contentBlocksStore)
          ->ObjectStore.createIndex(
              "forNote",
              "noteId",
              CreateIndexParams.make(~unique=false, ~multiEntry=false),
            )
          |> ignore;
        },
      )
    ->set(
        1,
        upgradeDb => {
          let tx = UpgradeDb.transaction(upgradeDb);

          Transaction.objectStore(tx, notesStore)
          ->ObjectStore.createIndex(
              "byUpdatedAt",
              "updatedAt",
              CreateIndexParams.make(~unique=false, ~multiEntry=false),
            )
          |> ignore;
        },
      )
  );

let runMigration = (upgradeDb, oldVersion) =>
  MigrationsMap.getExn(migrations, oldVersion, upgradeDb);

let runMigrations = (upgradeDb, oldVersion, newVersion) =>
  Belt.List.makeBy(newVersion - oldVersion, i => i + oldVersion)
  ->Belt.List.forEach(runMigration(upgradeDb));
