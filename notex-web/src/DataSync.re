type timerId;
[@bs.val] external setInterval: (unit => unit, int) => timerId = "";
[@bs.val] external clearInterval: timerId => unit = "";

type changeValue =
  | ContentBlock(Data.contentBlock);

type change = {
  id: string,
  change: changeValue,
};

let pendingChanges = Js.Dict.empty();

let unsafeDeleteKey: (Js.Dict.t(change), string) => unit = [%bs.raw
  {|
  function(dict,key){
     delete dict[key];
     return 0
   }
|}
];

let start = () =>
  setInterval(
    () => {
      Js.log2("Syncing changes:", pendingChanges);

      Js.Dict.values(pendingChanges)
      ->Belt.List.fromArray
      ->Belt.List.forEach(change =>
          switch (change.change) {
          | ContentBlock(contentBlock) =>
            Api.updateContentBlock(contentBlock)
            ->Future.get(result =>
                if (Belt.Result.isOk(result)) {
                  unsafeDeleteKey(pendingChanges, change.id);
                }
              )
          }
        );
    },
    5 * 1000,
  );

let pushContentBlock = (contentBlock: Data.contentBlock) => {
  let id = "contentBlock:" ++ string_of_int(contentBlock.id);
  let change = {id, change: ContentBlock(contentBlock)};

  Js.Dict.set(pendingChanges, id, change);
};
