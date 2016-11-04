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
    libogg \
    opus \
    opusfile

# not currently provided by homebrew:
# - gtest

wget https://github.com/google/googletest/archive/release-1.7.0.tar.gz -O gtest.tar.gz
tar zxf gtest.tar.gz
(cd googletest-release-1.7.0
    c++ -isystem include -I. -pthread -c src/gtest-all.cc
    c++ -isystem include -I. -pthread -c src/gtest_main.cc
    ar -rv /usr/local/lib/libgtest.a gtest-all.o
    ar -rv /usr/local/lib/libgtest_main.a gtest_main.o
    cp -R include/gtest /usr/local/include
)
