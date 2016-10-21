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
# - gtest

wget http://downloads.xiph.org/releases/opus/opus-1.1.tar.gz
tar zxf opus-1.1.tar.gz
(cd opus-1.1
    ./configure
    make
    make install
)

wget http://downloads.xiph.org/releases/opus/opusfile-0.6.tar.gz
tar zxf opusfile-0.6.tar.gz
(cd opusfile-0.6
    ./configure --disable-http
    make
    make install
)

wget https://github.com/google/googletest/archive/release-1.7.0.tar.gz -O gtest.tar.gz
tar zxf gtest.tar.gz
(cd googletest-release-1.7.0
    c++ -isystem include -I. -pthread -c src/gtest-all.cc
    c++ -isystem include -I. -pthread -c src/gtest_main.cc
    ar -rv /usr/local/lib/libgtest.a gtest-all.o
    ar -rv /usr/local/lib/libgtest_main.a gtest_main.o
    cp -R include/gtest /usr/local/include
)

# Replace SDL_opengl.h with the one from SDL 2.0.4
# TODO: remove this once 2.0.4 is (finally) released!
wget https://hg.libsdl.org/SDL/raw-file/c94127dcbe97/include/SDL_opengl.h -O /usr/local/include/SDL2/SDL_opengl.h
wget https://www.libsdl.org/tmp/SDL/include/SDL_opengl_glext.h -O /usr/local/include/SDL2/SDL_opengl_glext.h
