cmake_minimum_required (VERSION 2.6)
project (NarfBlock)

option(SERVER "Build NarfBlock server" ON)
option(CLIENT "Build NarfBlock client" ON)
option(TEST "Build NarfBlock unit tests" ON)

option(STATIC "Use static linking when building executables" OFF)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_RELEASE "")

if (NOT DEFINED PATCH_COMMAND)
  set(PATCH_COMMAND "patch")
endif()

if (NOT DEFINED GIT_COMMAND)
  set(GIT_COMMAND "git")
endif()

execute_process(COMMAND ${GIT_COMMAND} describe --tags --always --dirty OUTPUT_VARIABLE VERSION_REV OUTPUT_STRIP_TRAILING_WHITESPACE)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")
set(NARFBLOCK_DATA_PATH "${CMAKE_SOURCE_DIR}/../data/")

if (CLIENT)
	find_package (OpenGL REQUIRED)
	find_package (GLEW REQUIRED)
	find_package (SDL2 REQUIRED)
	find_package (Freetype REQUIRED)
	find_package (PNG REQUIRED)
endif ()

find_package (Threads REQUIRED)
find_package (ZLIB REQUIRED)
find_package (ENet REQUIRED)

if (SERVER)
	find_package (Curses REQUIRED)
endif ()

include (DownloadFont)

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	set(CMAKE_COMPILER_IS_CLANG 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	SET(CMAKE_C_FLAGS
		"-Wpointer-arith -Wtype-limits -Wwrite-strings -Wuninitialized -Werror=redundant-decls -Wsign-compare -Wconversion -g -fno-ident")
	SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=gnu++0x")
	if (STATIC)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
	endif()
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
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

if (CLIENT)
	# add dependencies to SDL libs for static linking
	if (WIN32)
		SET(SDL2_LIBRARY
			${SDL2_LIBRARY}
			imm32
			winmm
			version
			)
	endif()
endif ()

if (STATIC)
	add_definitions(-DGLEW_STATIC)
endif ()

if (CLIENT)
	include_directories (
		"${SDL2_INCLUDE_DIR}"
		"${FREETYPE_INCLUDE_DIRS}"
		"${GLEW_INCLUDE_DIR}"
		)
endif ()

if (SERVER)
	include_directories (
		"${CURSES_INCLUDE_DIR}"
		)
endif ()


include_directories (
	"${ENet_INCLUDE_DIRS}"
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_BINARY_DIR}"
	)

configure_file (
	"${PROJECT_SOURCE_DIR}/narf/version.h.in"
	"${PROJECT_BINARY_DIR}/narf/version.h"
	)

# generate C source file from binary to embed
FUNCTION(EMBED out_var in_f)
	SET(result)
	FILE(RELATIVE_PATH src_f_full ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
	get_filename_component(src_f ${src_f_full} NAME)
	get_filename_component(src_dir ${src_f_full} DIRECTORY)
	SET(out_f "${PROJECT_BINARY_DIR}/${src_f}.c")
	ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
		COMMAND xxd -i ${src_f} > ${out_f}
		DEPENDS ${in_f}
		WORKING_DIRECTORY ${src_dir}
		COMMENT "Embedding ${src_f}"
		VERBATIM
		)
	SET(${out_var} "${out_f}" PARENT_SCOPE)
ENDFUNCTION()


# source files used by both client and server
set (NARFBLOCK_COMMON_SOURCE_FILES
	narf/aabb.cpp
	narf/block.cpp
	narf/bytestream.cpp
	narf/chunk.cpp
	narf/entity.cpp
	narf/file.cpp
	narf/gameloop.cpp
	narf/ini.cpp
	narf/playercmd.cpp
	narf/texteditor.cpp
	narf/time.cpp
	narf/utf.cpp
	narf/world.cpp
	narf/cmd/cmd.cpp
	narf/math/floats.cpp
	narf/math/ints.cpp
	narf/util/tokenize.cpp
	narf/util/path.cpp
	narf/net/addr.cpp
	)


if (CLIENT)
	EMBED(embed_DroidSansMono_ttf ../data/DroidSansMono.ttf)
	EMBED(embed_terrain_png ../data/terrain.png)

	SET(NARFBLOCK_CLIENT_SOURCE_FILES
		narf/input.cpp
		narf/gl/context.cpp
		narf/gl/texture.cpp
		narf/gl/textureatlas.cpp
		narf/font.cpp
		narf/png.cpp
		narf/texture-font.cpp

		narf/client/main.cpp
		narf/client/console.cpp
		narf/client/renderer.cpp

		${embed_DroidSansMono_ttf}
		${embed_terrain_png}
		)

	if (WIN32)
		SET(NARFBLOCK_CLIENT_SOURCE_FILES
			${NARFBLOCK_CLIENT_SOURCE_FILES}
			narf/client/winresources.rc
			)
	endif()

	# client executable
	add_executable (narfblock
		# Mark as a "windowed" program (rather than console) on Windows
		# (no effect on other platforms)
		WIN32

		${NARFBLOCK_COMMON_SOURCE_FILES}
		${NARFBLOCK_CLIENT_SOURCE_FILES}
		)

	target_link_libraries (narfblock
		${FREETYPE_LIBRARY}
		${FREETYPE_GL_LIBRARY}
		${GLEW_LIBRARY}
		${OPENGL_LIBRARY}
		${SDL2_LIBRARY}
		${PNG_LIBRARY}
		${ZLIB_LIBRARY}
		${ENet_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		)

	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		set_target_properties (narfblock
			PROPERTIES LINK_FLAGS "-Wl,-Map=narfblock.map"
			)
	endif ()
endif ()

if (SERVER)
	# server executable
	add_executable (narfblock-server
		${NARFBLOCK_COMMON_SOURCE_FILES}

		narf/cursesconsole.cpp
		narf/server/main.cpp
		)

	target_link_libraries (narfblock-server
		${CMAKE_THREAD_LIBS_INIT}
		${CURSES_LIBRARY}
		${ENet_LIBRARIES}
		)
endif ()

if (TEST)
	# unit test executable
	add_executable (narfblock-test
		${NARFBLOCK_COMMON_SOURCE_FILES}

		narf/stdioconsole.cpp
		narf/test/main.cpp
		)

	target_link_libraries (narfblock-test
		${ENet_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		)
endif ()

if (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
