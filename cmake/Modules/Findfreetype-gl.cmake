set (FREETYPE_GL_SRC
  freetype-gl.h      vec234.h
  texture-atlas.cpp  texture-atlas.h
  texture-font.cpp   texture-font.h
  vector.cpp         vector.h
  opengl.h)


set(FREETYPE_GL_SRC_LIST ${FREETYPE_GL_SRC})
set(FREETYPE_GL_DIR "${CMAKE_SOURCE_DIR}/freetype-gl")

add_subdirectory("${FREETYPE_GL_DIR}" "${FREETYPE_GL_DIR}")

set(FREETYPE_GL_INCLUDE_DIR "${FREETYPE_GL_DIR}")

