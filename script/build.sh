#!/usr/bin/env sh
echo "Building frontend..."
(cd frontend && yarn build)

echo "Copying assets..."
(cd frontend/build && \
  tar cf - \
    --exclude=**.map \
.) | (cd notix-server/assets && tar xvf - )

echo "Building binary"
(cd notix-server && cargo build --release --features=embedded_assets)
