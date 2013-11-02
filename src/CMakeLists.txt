cmake_minimum_required (VERSION 2.6)
project (NarfBlock)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_RELEASE "")

if (NOT DEFINED PATCH_COMMAND)
  set(PATCH_COMMAND "patch")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")
set(NARFBLOCK_DATA_PATH "${CMAKE_SOURCE_DIR}/../data/")

find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)
find_package (SDL2 REQUIRED)
find_package (SDL2_image REQUIRED)
find_package (freetype-gl REQUIRED)
find_package (Freetype REQUIRED)
find_package (Threads REQUIRED)
find_package (Curses REQUIRED)
find_package (Poco REQUIRED)
find_package (PNG REQUIRED)
find_package (ZLIB REQUIRED)

include (DownloadFont)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_C_FLAGS
		"-Wpointer-arith -Wtype-limits -Wwrite-strings -Wuninitialized -Wno-pmf-conversions -Werror=redundant-decls -Wsign-compare -Wconversion -g -Wl,--as-needed -fno-ident")
	SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++0x")
	if (WIN32)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
	endif ()
endif ()

if (MSVC)
	# ask MSVC to define M_PI and related constants
	add_definitions( -D_USE_MATH_DEFINES )
endif ()

# figure out the correct header filename for curses
if (CURSES_HAVE_CURSES_H)
	add_definitions( -DHAVE_CURSES_H )
endif()
if (CURSES_HAVE_NCURSES_H)
	add_definitions( -DHAVE_NCURSES_H )
endif()
if (CURSES_HAVE_NCURSES_NCURSES_H)
	add_definitions( -DHAVE_NCURSES_NCURSES_H )
endif()
if (CURSES_HAVE_NCURSES_CURSES_H)
	add_definitions( -DHAVE_NCURSES_CURSES_H )
endif()

# add dependencies to SDL libs for static linking
if (WIN32)
	SET(SDL2_LIBRARY
		${SDL2_LIBRARY}
		imm32
		winmm
		version
		)
endif()

# add dependencies to SDL2_image libs for static linking
SET(SDL2IMAGE_LIBRARY
	${SDL2IMAGE_LIBRARY}
	${PNG_LIBRARY}
	${ZLIB_LIBRARY}
	)

if (WIN32)
	# assume libs are static linked on win32
	# TODO: make this configurable with cmake
	add_definitions(-DPOCO_STATIC)
	add_definitions(-DGLEW_STATIC)
endif()

include_directories (
	"${SDL2_INCLUDE_DIR}"
	"${SDL2IMAGE_INCLUDE_DIR}"
	"${FREETYPE_GL_INCLUDE_DIR}"
	"${FREETYPE_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIR}"
	"${CURSES_INCLUDE_DIR}"
	"${PocoFoundation_INCLUDE_DIR}"
	"${PocoUtil_INCLUDE_DIR}"
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_BINARY_DIR}"
	)

configure_file (
	"${PROJECT_SOURCE_DIR}/narf/version.h.in"
	"${PROJECT_BINARY_DIR}/narf/version.h"
	)

# source files used by both client and server
set (NARFBLOCK_COMMON_SOURCE_FILES
	narf/chunk.cpp
	narf/entity.cpp
	narf/world.cpp
	narf/config/config.cpp
	narf/math/floats.cpp
	narf/util/tokenize.cpp
	narf/util/path.cpp
	)

# client executable
add_executable (narfblock
	# Mark as a "windowed" program (rather than console) on Windows
	# (no effect on other platforms)
	WIN32

	${NARFBLOCK_COMMON_SOURCE_FILES}

	narf/input.cpp
	narf/texteditor.cpp
	narf/gl/context.cpp
	narf/gl/texture.cpp
	narf/font.cpp

	narf/client/main.cpp
	narf/client/chunk.cpp
	narf/client/console.cpp
	narf/client/world.cpp
	)

target_link_libraries (narfblock
	freetype-gl
	${PocoUtil_LIBRARY}
	${PocoFoundation_LIBRARY}
	${FREETYPE_LIBRARY}
	${FREETYPE_GL_LIBRARY}
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${SDL2IMAGE_LIBRARY}
	${SDL2_LIBRARY}
	# HACK: cmake's SDL_LIBRARY puts the libraries in the wrong order;
	# there are symbols in libSDLmain that depend on libSDL, but -lSDL is first in SDL_LIBRARY.
	# Add them again to get -lSDL after -lSDLmain.
	${SDL2_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	)

# server executable
add_executable (narfblock-server
	${NARFBLOCK_COMMON_SOURCE_FILES}

	narf/cursesconsole.cpp
	narf/server/main.cpp
	)

target_link_libraries (narfblock-server
	${PocoUtil_LIBRARY}
	${PocoFoundation_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${CURSES_LIBRARY}
	)

# unit test executable
add_executable (narfblock-test
	${NARFBLOCK_COMMON_SOURCE_FILES}

	narf/stdioconsole.cpp
	narf/test/main.cpp
	)

target_link_libraries (narfblock-test
	${PocoUtil_LIBRARY}
	${PocoFoundation_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	)

set(CMAKE_BUILD_TYPE Release)
