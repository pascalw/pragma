#!/usr/bin/env sh -e
RELEASE_DIR="$(dirname $0)/target/release"
APP_TEMPLATE="$(dirname $0)/macos/Pragma.app"
DESKTOP_BINARY="$(dirname $0)/../server/target/release/pragma-desktop"

pushd $(dirname $0)/../ >/dev/null
  BUILDER=local BIN=pragma-desktop FEATURES=desktop ./script/build.sh
popd >/dev/null

strip "$DESKTOP_BINARY"

mkdir -p "$RELEASE_DIR"
cp -fRp "$APP_TEMPLATE" "$RELEASE_DIR"

cp "$DESKTOP_BINARY" "$RELEASE_DIR/Pragma.app/Contents/MacOS/Pragma"
