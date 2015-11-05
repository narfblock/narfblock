#!/bin/sh

set -xe

os=$(uname)

case "$os" in
Linux)
    sudo add-apt-repository ppa:zoogie/sdl2-snapshots -y
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo add-apt-repository ppa:jblgf0/multimedia-precise -y
    sudo apt-get update -qq

    sudo apt-get install -y \
        cmake \
        libsdl2-dev \
        libpng12-dev \
        zlib1g-dev \
        libfreetype6-dev \
        libncurses5-dev \
        libenet-dev \
        libogg-dev \
        libopus-dev \
        libopusfile-dev \
        libgtest-dev \
        g++-4.8

    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 90

    # libgtest-dev just installs the source, so build it and install it system-wide
    cd /usr/src/gtest
    sudo cmake -DBUILD_SHARED_LIBS:BOOL=ON .
    sudo cmake --build .
    sudo mv libg*.so /usr/local/lib/
    ;;

Darwin)
    brew install \
        cmake \
        sdl2 \
        libpng \
        freetype \
        enet \
        libogg
    ;;

*)
    echo "unknown os $os"
    exit 1
    ;;
esac
