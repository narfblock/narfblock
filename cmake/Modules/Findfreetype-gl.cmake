

set (FREETYPE_GL_REVISION 226)
set (FREETYPE_GL_URL_BASE "https://freetype-gl.googlecode.com/svn-history/r${FREETYPE_GL_REVISION}/trunk/")
set (FREETYPE_GL_SRC
  freetype-gl.h      vec234.h
  markup.h
  texture-atlas.c    texture-atlas.h
  texture-font.c     texture-font.h
  font-manager.c     font-manager.h
  vector.c           vector.h
  platform.c         platform.h
  opengl.h
  mat4.c             mat4.h
  shader.c           shader.h
  vertex-buffer.c    vertex-buffer.h
  vertex-attribute.c vertex-attribute.h)


set(FREETYPE_GL_SRC_LIST ${FREETYPE_GL_SRC})
set(FREETYPE_GL_DIR "${CMAKE_BINARY_DIR}/freetype-gl")

if (NOT EXISTS "${FREETYPE_GL_DIR}/")
  message(STATUS "Creating ${FREETYPE_GL_DIR}")
  file (MAKE_DIRECTORY "${FREETYPE_GL_DIR}")
endif()


foreach (i IN LISTS FREETYPE_GL_SRC_LIST)
  if (NOT EXISTS "${FREETYPE_GL_DIR}/${i}")
		message(STATUS "Downloading ${i}")
    file(DOWNLOAD "${FREETYPE_GL_URL_BASE}${i}" "${FREETYPE_GL_DIR}/${i}")
  endif()
endforeach()

execute_process(COMMAND ${PATCH_COMMAND} --merge --binary -d "${FREETYPE_GL_DIR}" -i "${CMAKE_SOURCE_DIR}/../cmake/freetype-gl.platform.patch")

configure_file ("${CMAKE_SOURCE_DIR}/../cmake/freetype-gl.CMakeLists.txt" "${FREETYPE_GL_DIR}/CMakeLists.txt" COPYONLY)

add_subdirectory("${FREETYPE_GL_DIR}" "${FREETYPE_GL_DIR}")

set(FREETYPE_GL_INCLUDE_DIR "${FREETYPE_GL_DIR}")

