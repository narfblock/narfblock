if(NOT DEFINED PocoUtil_FOUND)

	set(POCO_UTIL_LIBRARY libPocoUtil.a)
	find_path(POCO_UTIL_INCLUDE_DIR Poco/Util/Util.h PATH "${PocoUtil_DIR}/include")
	execute_process(COMMAND ${CMAKE_COMMAND} -E echo "Meow: ${PATCH_COMMAND}")
	execute_process(COMMAND ${PATCH_COMMAND} --merge "../poco/Util/CMakeLists.txt" "../cmake/PocoUtil.CMakeLists.patch")
	include_directories("${POCO_UTIL_INCLUDE_DIR}")
	add_definitions( -DPOCO_STATIC -DPOCO_NO_AUTOMATIC_LIBS -DPOCO_UTIL_NO_JSONCONFIGURATION -DPOCO_UTIL_NO_JSONONFIGURATION -DPOCO_UTIL_NO_XMLCONFIGURATION)
	set( LIB_MODE STATIC )

	add_subdirectory(
		${PocoUtil_DIR}
		${CMAKE_BINARY_DIR}/PocoUtil
		)
endif()
