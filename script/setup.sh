#!/usr/bin/env sh
echo "Setting up pre-commit hook."
ln -sf ../../script/pre-commit.sh .git/hooks/pre-commit

(cd frontend && yarn install)
(cd notix-server && cargo build)
