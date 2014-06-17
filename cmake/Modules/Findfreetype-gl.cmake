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
set(FREETYPE_GL_DIR "${CMAKE_SOURCE_DIR}/freetype-gl")

add_subdirectory("${FREETYPE_GL_DIR}" "${FREETYPE_GL_DIR}")

set(FREETYPE_GL_INCLUDE_DIR "${FREETYPE_GL_DIR}")

