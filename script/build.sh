#!/usr/bin/env sh
BUILDER=${BUILDER:-local}

function rust_musl_builder() {
  docker run --rm -it -v "$(pwd)":/home/rust/src \
    -w /home/rust/src/notex-server \
    -v cargo-git:/home/rust/.cargo/git \
    -v cargo-registry:/home/rust/.cargo/registry \
    ekidd/rust-musl-builder "$@"
}

echo "Building frontend..."
(cd notex-web && yarn build)

echo "Copying assets..."
(cd notex-web/build && \
  tar cf - \
    --exclude=**.map \
.) | (rm -rf notex-server/assets && mkdir -p notex-server/assets && cd notex-server/assets && tar xvf - )

case "$BUILDER" in
  "local")
    echo "Building binary with local cargo"
    (cd notex-server && cargo build --release --features=embedded_assets)
    ;;
  "docker")
    echo "Building binary in Docker"
    (rust_musl_builder cargo build --release --features=embedded_assets)
    ;;
esac
