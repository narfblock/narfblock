#!/bin/sh

set -xe

# already installed/provided by system:
# - cmake
# - zlib
# - libpng

brew install \
    sdl2 \
    freetype \
    enet \
    libogg

# not currently provided by homebrew:
# - libopus
# - opusfile

wget http://downloads.xiph.org/releases/opus/opus-1.1.tar.gz
tar zxf opus-1.1.tar.gz
(cd opus-1.1
    ./configure
    make
    make install
)

wget http://downloads.xiph.org/releases/opus/opusfile-0.6.tar.gz
(cd opusfile-0.6
    ./configure
    make
    make install
)
