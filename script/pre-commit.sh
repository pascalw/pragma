#!/usr/bin/env bash
CHANGED_FILES=$(git diff --cached --name-status --diff-filter=ACM | awk '/\.rs$/ { print $2 }')

[ ! -z "$CHANGED_FILES" ] && {
  echo "Running rustfmt..."
  rustfmt --write-mode=overwrite $CHANGED_FILES

  echo "Running cargo check..."
  (cd notex-server && cargo check) || exit 1

  echo "Running clippy..."
  (cd notex-server && cargo +nightly clippy)
  git add $CHANGED_FILES
} || exit 0
