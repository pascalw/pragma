#!/usr/bin/env sh
CHANGED_FILES=$(git diff --cached --name-status --diff-filter=ACM | awk '/\.rs$/ { print $2 }')

[ ! -z "$CHANGED_FILES" ] && {
  echo "Running rustfmt..."
  rustfmt --write-mode=overwrite $CHANGED_FILES
  cargo +nightly clippy
  git add $CHANGED_FILES
} || exit 0
