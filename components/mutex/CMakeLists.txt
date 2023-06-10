# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE mutex_sources ${CMAKE_SOURCE_DIR}/components/mutex/src/*.*)

idf_component_register(
  SRCS ${mutex_sources}
  INCLUDE_DIRS "include"
)