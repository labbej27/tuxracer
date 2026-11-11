#!/bin/sh
# Builds SDL2, SDL2_mixer (MOD/.it via vendored libxmp) and Tcl 8.6 as
# static libraries into deps-prefix/<platform>.
# Usage: scripts/build-deps.sh macos|linux|windows
set -eu
PLATFORM=$1
ROOT=$(cd "$(dirname "$0")/.." && pwd)
DEPS=$ROOT/deps
PREFIX=$ROOT/deps-prefix/$PLATFORM
WORK=$ROOT/build-deps/$PLATFORM
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)
mkdir -p "$PREFIX" "$WORK"

CMAKE_EXTRA=""
case $PLATFORM in
  macos)
    export MACOSX_DEPLOYMENT_TARGET=11.0
    CMAKE_EXTRA="-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0"
    ;;
  windows)
    CMAKE_EXTRA="-DCMAKE_TOOLCHAIN_FILE=$ROOT/scripts/mingw-w64.cmake"
    ;;
esac

cm() { # src build extra-args...
  src=$1; bld=$2; shift 2
  cmake -S "$src" -B "$bld" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$PREFIX" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON $CMAKE_EXTRA "$@"
  cmake --build "$bld" -j "$JOBS"
  cmake --install "$bld"
}

# --- SDL2 -----------------------------------------------------------------
if [ ! -f "$PREFIX/.sdl2" ]; then
  cm "$DEPS/SDL2-2.32.10" "$WORK/sdl2" -DSDL_SHARED=OFF -DSDL_STATIC=ON \
     -DSDL_TEST=OFF -DSDL_TESTS=OFF
  touch "$PREFIX/.sdl2"
fi

# --- SDL2_mixer -----------------------------------------------------------
if [ ! -f "$PREFIX/.mixer" ]; then
  cm "$DEPS/SDL2_mixer-2.8.1" "$WORK/mixer" -DBUILD_SHARED_LIBS=OFF \
     -DSDL2MIXER_VENDORED=ON -DSDL2MIXER_DEPS_SHARED=OFF -DSDL2MIXER_SAMPLES=OFF \
     -DSDL2MIXER_MOD=ON -DSDL2MIXER_MOD_XMP=ON -DSDL2MIXER_MOD_MODPLUG=OFF \
     -DSDL2MIXER_WAVE=ON -DSDL2MIXER_VORBIS=STB -DSDL2MIXER_OPUS=OFF \
     -DSDL2MIXER_FLAC=OFF -DSDL2MIXER_MP3=OFF -DSDL2MIXER_MIDI=OFF \
     -DSDL2MIXER_WAVPACK=OFF -DSDL2MIXER_GME=OFF
  touch "$PREFIX/.mixer"
fi

# --- Tcl 8.6 (static, no init.tcl needed by the game) -----------------------
if [ ! -f "$PREFIX/.tcl" ]; then
  rm -rf "$WORK/tcl"; mkdir -p "$WORK/tcl"; cd "$WORK/tcl"
  case $PLATFORM in
    macos)
      CFLAGS="-O2 -arch arm64 -arch x86_64 -mmacosx-version-min=11.0" \
        "$DEPS/tcl8.6.16/unix/configure" --prefix="$PREFIX" --disable-shared \
        --disable-corefoundation --enable-threads
      ;;
    linux)
      CFLAGS="-O2 -fPIC" "$DEPS/tcl8.6.16/unix/configure" --prefix="$PREFIX" \
        --disable-shared --enable-threads
      ;;
    windows)
      "$DEPS/tcl8.6.16/win/configure" --prefix="$PREFIX" --host=x86_64-w64-mingw32 \
        --disable-shared --enable-64bit --enable-threads
      ;;
  esac
  make -j "$JOBS" binaries
  make install-binaries install-headers 2>/dev/null || make install-libraries install-headers
  touch "$PREFIX/.tcl"
fi
echo "deps OK -> $PREFIX"
