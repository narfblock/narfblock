#!/bin/sh

set -xe

add-apt-repository ppa:zoogie/sdl2-snapshots -y
add-apt-repository ppa:ubuntu-toolchain-r/test -y
add-apt-repository ppa:jblgf0/multimedia-precise -y
apt-get update -qq

apt-get install -y \
    cmake \
    libsdl2-dev \
    libpng12-dev \
    zlib1g-dev \
    libglew-dev \
    libfreetype6-dev \
    libncurses5-dev \
    libenet-dev \
    libogg-dev \
    libopus-dev \
    libopusfile-dev \
    libgtest-dev \
    g++-4.8

update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90

# libgtest-dev just installs the source, so build it and install it system-wide
cd /usr/src/gtest
cmake -DBUILD_SHARED_LIBS:BOOL=ON .
cmake --build .
mv libg*.so /usr/local/lib/
