open Belt;

let mapError = (result: Result.t('a, 'b), mapper: 'b => 'c): Result.t('a, 'c) =>
  switch (result) {
  | Result.Ok(value) => Belt.Result.Ok(value)
  | Result.Error(error) => Result.Error(mapper(error))
  };
