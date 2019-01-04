open Belt.Result;

let mapOk =
    (mapper: 'a => 'c, promise: Repromise.t(Belt.Result.t('a, 'b)))
    : Repromise.t(Belt.Result.t('c, 'b)) =>
  promise
  |> Repromise.map(
       fun
       | Ok(value) => Ok(mapper(value))
       | Error(_) as error => error,
     );

let tapOk =
    (tap: 'a => unit, promise: Repromise.t(Belt.Result.t('a, 'b)))
    : Repromise.t(Belt.Result.t('a, 'b)) =>
  mapOk(
    value => {
      tap(value);
      value;
    },
    promise,
  );

let flatMapOk =
    (
      mapper: 'a => Repromise.t(Belt.Result.t('c, 'b)),
      promise: Repromise.t(Belt.Result.t('a, 'b)),
    )
    : Repromise.t(Belt.Result.t('c, 'b)) =>
  promise
  |> Repromise.andThen(
       fun
       | Ok(value) => mapper(value)
       | Error(_) as error => Repromise.resolved(error),
     );

let mapSome = (mapper: 'a => 'b, promise: Repromise.t(option('a))): Repromise.t(option('b)) =>
  promise
  |> Repromise.map(
       fun
       | Some(value) => Some(mapper(value))
       | None => None,
     );

let toResultPromise =
    (promise: Js.Promise.t('a)): Repromise.t(Belt.Result.t('a, Js.Promise.error)) =>
  promise
  |> Repromise.Rejectable.fromJsPromise
  |> Repromise.Rejectable.map(value => Belt.Result.Ok(value))
  |> Repromise.Rejectable.catch(error => Repromise.resolved(Belt.Result.Error(error)));
