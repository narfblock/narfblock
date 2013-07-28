cmake_minimum_required (VERSION 2.8.5)
find_package (OpenGL REQUIRED)
find_package (GLEW REQUIRED)
find_package (Freetype REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}
                     ${GLEW_INCLUDE_DIR}
                     ${FREETYPE_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR} )

add_library (freetype-gl STATIC ${FREETYPE_GL_SRC})

# vim: ft=cmake
