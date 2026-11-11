#!/bin/sh
# Run inside the tuxracer-build container: builds the game for linux|windows
set -eu
P=$1; shift
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# Build outputs live outside the source tree (which may be synced by iCloud)
OUT=${TR_BUILD_ROOT:-$ROOT/out}
EXTRA=""
[ "$P" = windows ] && EXTRA="-DCMAKE_TOOLCHAIN_FILE=$ROOT/scripts/mingw-w64.cmake"
cmake -S "$ROOT" -B "$OUT/build-$P" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$OUT/deps-prefix/$P" -DTCL_PREFIX="$OUT/deps-prefix/$P" $EXTRA "$@"
cmake --build "$OUT/build-$P"
