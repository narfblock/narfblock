cmake_minimum_required (VERSION 2.6)
project (narflib)

get_directory_property(hasParent PARENT_DIRECTORY)

find_package (ZLIB REQUIRED)

set(NARFLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(NARFLIB_SOURCE_DIR ${PROJECT_SOURCE_DIR})

if (hasParent)
	set(STATIC ON)
	set(NARFLIB_INCLUDE_DIRS ${NARFLIB_INCLUDE_DIRS} PARENT_SCOPE)
	set(NARFLIB_SOURCE_DIR ${NARFLIB_SOURCE_DIR} PARENT_SCOPE)
else()

	option(STATIC "Use static linking when building executables" OFF)
	option(STATIC_LIBSTDCXX "Static link libstdc++" OFF)
	option(STATIC_LIBGCC "Static link libgcc" OFF)

	if (STATIC)
		set(STATIC_LIBSTDCXX ON)
		set(STATIC_LIBGCC ON)
	endif()

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
	if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
		set(CMAKE_COMPILER_IS_CLANG 1)
	endif ()

	if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
		SET(CMAKE_C_FLAGS
			"${CMAKE_C_FLAGS} -Wpointer-arith -Wtype-limits -Wwrite-strings -Wuninitialized -Werror=redundant-decls -Wsign-compare -Wconversion -g -fno-ident")

		# don't export symbols (especially for PIC builds, but it doesn't hurt elsewhere)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

		if (STATIC)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
		endif()

		if (STATIC_LIBGCC)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
		endif()

		if (STATIC_LIBSTDCXX)
			SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libstdc++")
		endif()

		if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
			# position-independent executable
			if (PIE)
				SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
				SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,-z,relro,-z,now")
			endif()

			SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
		endif()


		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=gnu++0x")
	endif ()

	if (MSVC)
		# ask MSVC to define M_PI and related constants
		add_definitions( -D_USE_MATH_DEFINES )
	endif ()
endif()

# generate C source file from binary to embed
FUNCTION(EMBED out_var in_f)
	SET(result)
	FILE(RELATIVE_PATH src_f_full ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
	get_filename_component(src_f ${src_f_full} NAME)
	get_filename_component(src_dir ${src_f_full} PATH)
	SET(out_f "${PROJECT_BINARY_DIR}/${src_f}.cpp")
	ADD_CUSTOM_COMMAND(OUTPUT ${out_f}
		COMMAND ${NARFLIB_SOURCE_DIR}/embed.sh ${src_f} ${out_f}
		DEPENDS ${in_f}
		WORKING_DIRECTORY ${src_dir}
		COMMENT "Embedding ${src_f}"
		VERBATIM
		)
	SET(${out_var} "${out_f}" PARENT_SCOPE)
ENDFUNCTION()

set (NARFLIB_SOURCE_FILES
	bytestream.cpp
	console.cpp
	embed.cpp
	file.cpp
	ini.cpp
	stdioconsole.cpp
	tokenize.cpp
	utf.cpp
	)

include_directories (
	"${NARFLIB_INCLUDE_DIRS}"
	"${ZLIB_INCLUDE_DIRS}"
	)

if(STATIC)
	add_library (narflib STATIC ${NARFLIB_SOURCE_FILES})
else()
	add_library (narflib SHARED ${NARFLIB_SOURCE_FILES})
	target_link_libraries (narflib ${ZLIB_LIBRARY})
endif()

