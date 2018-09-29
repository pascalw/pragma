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
  Js.Global.setInterval(
    () =>
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
        ),
    5 * 1000,
  );

let pushContentBlock = (contentBlock: Data.contentBlock) => {
  let id = "contentBlock:" ++ contentBlock.id;
  let change = {id, change: ContentBlock(contentBlock)};

  Js.Dict.set(pendingChanges, id, change);
};
