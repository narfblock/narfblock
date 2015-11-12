cmake_minimum_required (VERSION 2.6)
project (narflib)

find_package (ZLIB REQUIRED)

set (NARFLIB_SOURCE_FILES
	console.cpp
	embed.cpp
	file.cpp
	ini.cpp
	stdioconsole.cpp
	tokenize.cpp
	utf.cpp
	)

set (NARFLIB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set (NARFLIB_INCLUDE_DIRS ${NARFLIB_INCLUDE_DIRS} PARENT_SCOPE)

set (NARFLIB_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set (NARFLIB_SOURCE_DIR ${NARFLIB_SOURCE_DIR} PARENT_SCOPE)

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

include_directories (
	"${NARFLIB_INCLUDE_DIRS}"
	"${ZLIB_INCLUDE_DIRS}"
	)

add_library (narflib STATIC ${NARFLIB_SOURCE_FILES})
