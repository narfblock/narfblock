if(NOT DEFINED PocoFoundation_FOUND)

	set(POCO_FOUNDATION_LIBRARY libPocoFoundation.a)
	find_path(POCO_FOUNDATION_INCLUDE_DIR Poco/Foundation.h PATH "${PocoFoundation_DIR}/include")
	# Error.cpp doesn't build on linux, but isn't needed by anything either.
	# So let's just blank the file so it'll compile.
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove -f "${PocoFoundation_DIR}/src/Error.cpp")
	execute_process(COMMAND ${CMAKE_COMMAND} -E touch "${PocoFoundation_DIR}/src/Error.cpp")
	include_directories("${POCO_FOUNDATION_INCLUDE_DIR}")

	add_subdirectory(
		${PocoFoundation_DIR}
		${CMAKE_BINARY_DIR}/PocoFoundation
		)
endif()
