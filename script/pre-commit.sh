#!/usr/bin/env bash
CHANGED_RUST_FILES=$(git diff --cached --name-only --diff-filter=ACM "*.rs" | tr '\n' ' ')
CHANGED_REASON_FILES=$(git diff --cached --name-only --diff-filter=ACM "*.re" | tr '\n' ' ')

[ ! -z "$CHANGED_RUST_FILES" ] && {
  echo "Running rustfmt..."
  rustfmt --write-mode=overwrite "$CHANGED_RUST_FILES"

  echo "Running cargo check..."
  (cd notex-server && cargo check >/dev/null 2>&1) || exit 1

  echo "Running clippy..."
  (cd notex-server && cargo +nightly clippy)
  echo "$CHANGED_RUST_FILES" | xargs git add
}

[ ! -z "$CHANGED_REASON_FILES" ] && {
  echo "Compiling..."
  (cd notex-web && bsb -clean -make-world) || exit 1

  echo "Running refmt..."
  echo "$CHANGED_REASON_FILES" | xargs -L1 refmt --in-place

  echo "$CHANGED_REASON_FILES" | xargs git add
} || exit 0
