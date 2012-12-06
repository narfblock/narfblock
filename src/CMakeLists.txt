cmake_minimum_required (VERSION 2.6)
project (Narfblock)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 0)
set (VERSION_RELEASE "")

configure_file (
  "${PROJECT_SOURCE_DIR}/test.h.in"
  "${PROJECT_BINARY_DIR}/test.h"
  )

add_executable(test test.cpp)
