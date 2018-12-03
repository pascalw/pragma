#!/usr/bin/env sh -e

function rust_musl_builder() {
  docker run --rm -it -v "$(pwd)":/home/rust/src \
    -w /home/rust/src/server \
    -v cargo-git:/home/rust/.cargo/git \
    -v cargo-registry:/home/rust/.cargo/registry \
    ekidd/rust-musl-builder "$@"
}

function build_frontend() {
  echo "Building frontend..."
  (cd web && yarn build)

  echo "Copying assets..."
  (cd web/build && \
    tar cf - \
  .) | (rm -rf server/assets && mkdir -p server/assets && cd server/assets && tar xvf - )
}

case "$BUILDER" in
  "local")
    build_frontend
    echo "Building binary with local cargo"
    (cd server && cargo build --release --features=embedded_assets)
    ;;
  "docker")
    build_frontend
    echo "Building binary in Docker"
    (rust_musl_builder sh -c '\
      cargo build --release --features=embedded_assets \
      && strip target/x86_64-unknown-linux-musl/release/pragma -o \
      target/x86_64-unknown-linux-musl/release/pragma.stripped')
    ;;
esac

case "$TARGET" in
  "docker-image")
    echo "Building Docker image"
    docker build -t pragma:$(git rev-parse --short HEAD) -t pragma:latest .
    ;;
esac
