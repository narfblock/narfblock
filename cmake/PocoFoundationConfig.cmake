if(NOT DEFINED PocoFoundation_FOUND)

	set(POCO_FOUNDATION_LIBRARY libPocoFoundation.a)
	find_path(POCO_FOUNDATION_INCLUDE_DIR Poco/Foundation.h PATH "${PocoFoundation_DIR}/include")
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "Meow: ${PATCH_COMMAND}")
	execute_process(COMMAND ${PATCH_COMMAND} --merge "../poco/Foundation/CMakeLists.txt" "../cmake/PocoFoundation.CMakeLists.patch")
	include_directories("${POCO_FOUNDATION_INCLUDE_DIR}")
	add_definitions( -DPOCO_STATIC -DPOCO_NO_AUTOMATIC_LIBS)
	set( LIB_MODE STATIC )

	add_subdirectory(
		${PocoFoundation_DIR}
		${CMAKE_BINARY_DIR}/PocoFoundation
		)
endif()
