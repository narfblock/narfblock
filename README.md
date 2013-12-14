Narfblock
=========

# Building

## Prerequisites

- CMake 2.6 or newer
- libSDL 2.0 development libraries
- libSDL2_image development libraries
- GLEW development libraries
- FreeType development libraries
- ncurses (Linux) or pdcurses (Windows) development libraries

## Build Commands

First get a copy of the source:

	git clone https://github.com/narfblock/narfblock.git
	cd narfblock/build

Then generate a Makefile using CMake and perform the build:

	cmake ../src && make

Or on Windows with MinGW in an MSYS shell:

	cmake -G "MSYS Makefiles" ../src && make


## Credits

Textures from the Isabella II texture pack by Bonemouse are used: http://www.minecraftforum.net/topic/242175-Isabella/
