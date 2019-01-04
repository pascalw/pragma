#!/usr/bin/env bash
CHANGED_RUST_FILES=$(git diff --cached --name-only --diff-filter=ACM "*.rs" | tr '\n' ' ')
CHANGED_REASON_FILES=$(git diff --cached --name-only --diff-filter=ACM "*.re" | tr '\n' ' ')

[ ! -z "$CHANGED_RUST_FILES" ] && {
  echo "Running rustfmt..."
  (cd server && cargo fmt)

  echo "Running clippy..."
  (cd server && cargo clippy --all-targets --all-features -- -D warnings)
  echo "$CHANGED_RUST_FILES" | xargs git add
}

[ ! -z "$CHANGED_REASON_FILES" ] && {
  echo "Compiling..."
  (cd web && yarn compile) || exit 1

  echo "Running refmt..."
  echo "$CHANGED_REASON_FILES" | xargs ./web/node_modules/.bin/bsrefmt --in-place --print-width=100

  echo "$CHANGED_REASON_FILES" | xargs git add
} || exit 0
