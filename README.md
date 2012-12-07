Narfblock
=========

# Building

## Prerequisites

- CMake 2.6 or newer
- libSDL 1.2 development libraries

## Build Commands

First get a copy of the source:

	git clone https://github.com/narfblock/narfblock.git
	cd narfblock/build

Then generate a Makefile using CMake and perform the build:

	cmake ../src && make

Or on Windows with MinGW in an MSYS shell:

	cmake -G "MSYS Makefiles" ../src && make
