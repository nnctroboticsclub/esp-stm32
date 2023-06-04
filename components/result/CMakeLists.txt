# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE result_sources ${CMAKE_SOURCE_DIR}/components/result/src/*.*)

idf_component_register(
  SRCS ${result_sources}
  INCLUDE_DIRS "include"
)