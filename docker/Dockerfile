# Build environment for the Linux x86-64 and Windows (MinGW-w64) versions.
# Ubuntu 22.04 keeps the glibc requirement low (2.35) for wide compatibility.
FROM --platform=linux/amd64 ubuntu:22.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build pkg-config ca-certificates git file \
      libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxinerama-dev \
      libxss-dev libxxf86vm-dev libxfixes-dev libwayland-dev libxkbcommon-dev \
      wayland-protocols libegl1-mesa-dev libgl1-mesa-dev libglu1-mesa-dev \
      libasound2-dev libpulse-dev libudev-dev libdbus-1-dev zlib1g-dev \
      g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64 \
      xvfb libgl1-mesa-dri xauth \
  && rm -rf /var/lib/apt/lists/*
