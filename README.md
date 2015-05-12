Narfblock
=========

[![Build Status](https://travis-ci.org/narfblock/narfblock.svg?branch=master)](https://travis-ci.org/narfblock/narfblock)

# Building

## Prerequisites

- CMake 2.6 or newer
- libSDL 2.0 development libraries
- libpng development libraries
- zlib development libraries
- FreeType development libraries
- ncurses (Linux) or pdcurses (Windows) development libraries
- ENet 1.3 or newer development libraries
- GoogleTest (gtest) development libraries

## Build Commands

First get a copy of the source:

	git clone https://github.com/narfblock/narfblock.git
	cd narfblock/build

Then generate a Makefile using CMake and perform the build:

	cmake ../src && make

Or on Windows with MinGW in an MSYS shell:

	cmake -G "MSYS Makefiles" ../src && make

## Cross-Compiling for Windows on Linux

First install a suitable MinGW-w64 cross-compiler toolchain.
On Debian, this is provided by the mingw-w64 package.

The easiest way to get the required dependencies for MinGW on Linux is to build them using the scripts provided in the deps directory:

	(cd deps && ./buildall)

Then point CMake at the generated toolchain file and configuration (example to compile for 32-bit Windows; substitute x86\_64 instead of i686 to compile for 64-bit Windows):

	(cd build && \
		cmake \
			-DSTATIC:BOOL=ON \
			-DCMAKE_TOOLCHAIN_FILE=../deps/i686-w64-mingw32/cmake-toolchain \
			-C ../deps/i686-w64-mingw32/deps.cmake ../src && \
			make)

## Enabling Debug

By default, CMake selects a release build; to build for debug:

	(cd build && cmake -DCMAKE_BUILD_TYPE=Debug ../src && make)

## Disabling Server or Client Build

To disable components, specify additional options on the cmake command line:

	(cd build && cmake -DSERVER:BOOL=OFF ../src) # disable server; build client
	(cd build && cmake -DCLIENT:BOOL=OFF ../src) # disable client; build server

## Credits

Textures from the Isabella II texture pack by Bonemouse are used: http://www.minecraftforum.net/topic/242175-Isabella/
