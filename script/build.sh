#!/usr/bin/env sh
echo "Building frontend..."
(cd notex-web && yarn build)

echo "Copying assets..."
(cd notex-web/build && \
  tar cf - \
    --exclude=**.map \
.) | (cd notex-server/assets && tar xvf - )

echo "Building binary"
(cd notex-server && cargo build --release --features=embedded_assets)
