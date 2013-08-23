if(NOT DEFINED PocoFoundation_FOUND)

	set(POCO_FOUNDATION_LIBRARY libPocoFoundation.a)
	find_path(POCO_FOUNDATION_INCLUDE_DIR Poco/Foundation.h PATH "${PocoFoundation_DIR}/include")
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "Meow: ${PATCH_COMMAND}")
	execute_process(COMMAND ${PATCH_COMMAND} --merge "../poco/Foundation/CMakeLists.txt" "../cmake/PocoFoundation.CMakeLists.patch")
	execute_process(COMMAND ${PATCH_COMMAND} --merge "../poco/Foundation/include/Poco/FPEnvironment_WIN32.h" "../cmake/PocoFoundation.FPEnvironment_WIN32.patch")
	execute_process(COMMAND ${PATCH_COMMAND} --merge "../poco/Foundation/src/Environment_WIN32U.cpp" "../cmake/PocoFoundation.Environment_WIN32U.patch")
	include_directories("${POCO_FOUNDATION_INCLUDE_DIR}")
	add_definitions( -DPOCO_STATIC -DPOCO_NO_AUTOMATIC_LIBS)
	set( LIB_MODE STATIC )

	add_subdirectory(
		${PocoFoundation_DIR}
		${CMAKE_BINARY_DIR}/PocoFoundation
		)
endif()
