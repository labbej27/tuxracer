#!/bin/sh
# Assembles the three release archives from the build outputs.
# Usage: TR_BUILD_ROOT=... scripts/package.sh <dest-dir>
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${TR_BUILD_ROOT:-$ROOT/out}
DEST=$1
V=0.61-sdl2
STAGE=$OUT/stage; rm -rf "$STAGE"; mkdir -p "$STAGE" "$DEST"
docs() { cp "$ROOT/packaging/LISEZMOI.txt" "$ROOT/COPYING" "$ROOT/AUTHORS" "$1/"; }

# Source archive (GPL: ships with the binaries)
git -C "$ROOT" archive --format=tar.gz --prefix="tuxracer-$V-src/" \
    -o "$DEST/tuxracer-$V-src.tar.gz" HEAD

# macOS (universal, ad-hoc signed)
M=$STAGE/TuxRacer-$V-macOS; mkdir -p "$M"
cp -R "$OUT/build-macos/Tux Racer.app" "$M/"
codesign --force --deep -s - "$M/Tux Racer.app"
docs "$M"
(cd "$STAGE" && ditto -c -k --keepParent "TuxRacer-$V-macOS" "$DEST/TuxRacer-$V-macOS.zip")

# Windows x64
W=$STAGE/TuxRacer-$V-Windows-x64; mkdir -p "$W"
cp "$OUT/tuxracer.exe" "$W/"; cp -R "$ROOT/data" "$W/data"; docs "$W"
(cd "$STAGE" && zip -qr -X "$DEST/TuxRacer-$V-Windows-x64.zip" "TuxRacer-$V-Windows-x64")

# Linux x86-64
L=$STAGE/TuxRacer-$V-Linux-x86_64; mkdir -p "$L"
cp "$OUT/tuxracer.linux" "$L/tuxracer"; chmod 755 "$L/tuxracer"
cp -R "$ROOT/data" "$L/data"; docs "$L"
cp "$ROOT/packaging/linux/tuxracer.desktop" "$ROOT/packaging/tuxracer.png" "$L/"
(cd "$STAGE" && COPYFILE_DISABLE=1 tar --no-xattrs -czf "$DEST/TuxRacer-$V-Linux-x86_64.tar.gz" "TuxRacer-$V-Linux-x86_64")

ls -la "$DEST"
