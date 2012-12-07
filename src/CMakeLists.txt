cmake_minimum_required (VERSION 2.6)
project (NarfBlock)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_RELEASE "")

find_package (OpenGL REQUIRED)
find_package (SDL REQUIRED)

include_directories (
	# cmake's SDL_INCLUDE_DIR incorrectly includes the actual SDL dir;
	# append .. to work around this
	"${SDL_INCLUDE_DIR}/.."
	)

configure_file (
	"${PROJECT_SOURCE_DIR}/test.h.in"
	"${PROJECT_BINARY_DIR}/test.h"
	)

add_executable (narfblock
	# Mark as a "windowed" program (rather than console) on Windows
	# (no effect on other platforms)
	WIN32

	test.cpp)

target_link_libraries (narfblock
	${OPENGL_LIBRARY}
	${SDL_LIBRARY}
	# HACK: cmake's SDL_LIBRARY puts the libraries in the wrong order;
	# there are symbols in libSDLmain that depend on libSDL, but -lSDL is first in SDL_LIBRARY.
	# Add them again to get -lSDL after -lSDLmain.
	${SDL_LIBRARY}
	)
