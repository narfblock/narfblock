cmake_minimum_required (VERSION 2.6)
project (NarfBlock)

option(SERVER "Build NarfBlock server" ON)
option(CLIENT "Build NarfBlock client" ON)
option(TEST "Build NarfBlock unit tests" ON)

option(STATIC "Use static linking when building executables" OFF)

option(PIE "Create position-independent executables" ON)

if (PIE AND STATIC)
	# PIE + STATIC will result in TEXTRELs unless all libraries have been
	# built for PIC/PIE, so don't allow it
	set(PIE OFF)
endif()

# -pie with MinGW-w64 breaks startup (incorrect entry point
# is used), so don't allow PIE on Windows for now
# http://sourceforge.net/p/mingw-w64/mailman/mingw-w64-public/thread/CAGFXeQKg0J%2Bax0Dj-cBXGGrwWT15K-aRsWWaBN6wWSDUtRHoLg@mail.gmail.com/
if (PIE AND WIN32)
	set(PIE OFF)
endif()

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

# generate list of authors separated by C newlines
execute_process(COMMAND ${GIT_COMMAND} shortlog -sn HEAD OUTPUT_VARIABLE VERSION_AUTHORS OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "[ ]+[0-9]+\t" "" VERSION_AUTHORS ${VERSION_AUTHORS})
string(REGEX REPLACE "\nnarfblock" "" VERSION_AUTHORS ${VERSION_AUTHORS})
string(REPLACE "\n" "\\n" VERSION_AUTHORS ${VERSION_AUTHORS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")
set(NARFBLOCK_DATA_PATH "${CMAKE_SOURCE_DIR}/../data/")

if (CLIENT)
	find_package (OpenGL REQUIRED)
	find_package (SDL2 REQUIRED)
	find_package (Freetype REQUIRED)
	find_package (PNG REQUIRED)
	find_package (OpusFile REQUIRED)
	find_package (Opus REQUIRED)
	find_package (LibOgg REQUIRED)
endif ()

if (TEST)
	find_package (GTest REQUIRED)
endif ()

find_package (Threads REQUIRED)
find_package (ZLIB REQUIRED)
find_package (ENet REQUIRED)

if (SERVER)
	find_package (Curses REQUIRED)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	set(CMAKE_COMPILER_IS_CLANG 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
	SET(CMAKE_C_FLAGS
		"${CMAKE_C_FLAGS} -Wpointer-arith -Wtype-limits -Wwrite-strings -Wuninitialized -Werror=redundant-decls -Wsign-compare -Wconversion -g -fno-ident")

	# don't export symbols (especially for PIC builds, but it doesn't hurt elsewhere)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

	if (STATIC)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc -static-libstdc++")
	endif()

	# position-independent executable
	if (PIE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
		SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,-z,relro,-z,now")
	endif()

	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=gnu++0x")
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

if (CLIENT)
	include_directories (
		"${SDL2_INCLUDE_DIR}"
		"${FREETYPE_INCLUDE_DIRS}"
		"${OpusFile_INCLUDE_DIRS}"
		"${Opus_INCLUDE_DIRS}"
		"${LibOgg_INCLUDE_DIRS}"
		)
endif ()

if (SERVER)
	include_directories (
		"${CURSES_INCLUDE_DIR}"
		)
endif ()

if (TEST)
	include_directories (
		"${GTEST_INCLUDE_DIRS}"
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
	get_filename_component(src_dir ${src_f_full} PATH)
	SET(out_f "${PROJECT_BINARY_DIR}/${src_f}.cpp")
	ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
		COMMAND ${CMAKE_SOURCE_DIR}/embed.sh ${src_f} ${out_f}
		DEPENDS ${in_f}
		WORKING_DIRECTORY ${src_dir}
		COMMENT "Embedding ${src_f}"
		VERBATIM
		)
	SET(${out_var} "${out_f}" PARENT_SCOPE)
ENDFUNCTION()

EMBED(embed_extra_credits_txt ../extra-credits.txt)

# source files used by both client and server
set (NARFBLOCK_COMMON_SOURCE_FILES
	narf/aabb.cpp
	narf/block.cpp
	narf/bytestream.cpp
	narf/chunk.cpp
	narf/embed.cpp
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
	narf/net/socket.cpp
	${embed_extra_credits_txt}
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
		${OPENGL_LIBRARY}
		${SDL2_LIBRARY}
		${PNG_LIBRARY}
		${ZLIB_LIBRARY}
		${ENet_LIBRARIES}
		${OpusFile_LIBRARIES}
		${Opus_LIBRARIES}
		${LibOgg_LIBRARIES}
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
		${ZLIB_LIBRARY}
		)
endif ()

if (TEST)
	# unit test executable
	file(GLOB_RECURSE NARFBLOCK_TEST_SOURCE_FILES ${PROJECT_SOURCE_DIR}/narf/test/*.cpp)
	add_executable (narfblock-test
		${NARFBLOCK_COMMON_SOURCE_FILES}
		${NARFBLOCK_TEST_SOURCE_FILES}
		narf/stdioconsole.cpp
		)

	target_link_libraries (narfblock-test
		${ENet_LIBRARIES}
		${GTEST_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		${ZLIB_LIBRARY}
		)
endif ()

if (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
		FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
