cmake_minimum_required (VERSION 2.6)
project (NarfBlock)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_RELEASE "")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")
set(NARFBLOCK_DATA_PATH "${CMAKE_SOURCE_DIR}/../data/")

find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)
find_package (SDL REQUIRED)
find_package (SDL_image REQUIRED)
find_package (YamlCpp REQUIRED)
find_package (freetype-gl REQUIRED)
find_package (Freetype REQUIRED)

include (DownloadFont)

# use Boost autolinking instead of explicit package names on MSVC
# CMake doesn't generate the correct filenames for the Boost MSVC libraries
if (MSVC)
	find_package (Boost REQUIRED)
else ()
	find_package (Boost REQUIRED COMPONENTS filesystem)
endif ()


if (CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS "-std=gnu++0x -Wall -Wno-switch -g")
endif ()

if (MSVC)
	# ask MSVC to define M_PI and related constants
	add_definitions( -D_USE_MATH_DEFINES )
endif ()

include_directories (
	# cmake's SDL_INCLUDE_DIR incorrectly includes the actual SDL dir;
	# append .. to work around this
	"${SDL_INCLUDE_DIR}/.."
	"${Boost_INCLUDE_DIR}"
	"${YAMLCPP_INCLUDE_DIR}"
	"${FREETYPE_GL_INCLUDE_DIR}"
	"${FREETYPE_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIR}"
	"${PROJECT_SOURCE_DIR}"
	)

configure_file (
	"${PROJECT_SOURCE_DIR}/test.h.in"
	"${PROJECT_BINARY_DIR}/test.h"
	)

# source files used by both client and server
set (NARFBLOCK_COMMON_SOURCE_FILES
	narf/config/configmanager.cpp
	narf/config/yamlmanager.cpp
	narf/config/property.cpp
	narf/util/tokenize.cpp
	)

# client executable
add_executable (narfblock
	# Mark as a "windowed" program (rather than console) on Windows
	# (no effect on other platforms)
	WIN32

	${NARFBLOCK_COMMON_SOURCE_FILES}

	narf/chunk.cpp
	narf/entity.cpp
	narf/input.cpp
	narf/world.cpp
	narf/gl/context.cpp
	narf/gl/texture.cpp
	narf/font.cpp
	narf/math/floats.cpp

	narf/client/main.cpp
	narf/client/chunk.cpp
	narf/client/world.cpp
	)

target_link_libraries (narfblock
	freetype-gl
	${YAMLCPP_LIBRARY}
	${FREETYPE_LIBRARY}
	${FREETYPE_GL_LIBRARY}
	${GLEW_LIBRARY}
	${OPENGL_LIBRARY}
	${SDL_IMAGE_LIBRARY}
	${SDL_LIBRARY}
	# HACK: cmake's SDL_LIBRARY puts the libraries in the wrong order;
	# there are symbols in libSDLmain that depend on libSDL, but -lSDL is first in SDL_LIBRARY.
	# Add them again to get -lSDL after -lSDLmain.
	${SDL_LIBRARY}
	${Boost_LIBRARIES}
	)

# server executable
add_executable (narfblock-server
	${NARFBLOCK_COMMON_SOURCE_FILES}

	narf/server/main.cpp
	)

target_link_libraries (narfblock-server
	${YAMLCPP_LIBRARY}
	${Boost_LIBRARIES}
	)

set(CMAKE_BUILD_TYPE Release)
