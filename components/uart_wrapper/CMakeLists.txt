# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE uart_wrapper ${CMAKE_SOURCE_DIR}/components/uart_wrapper/src/*.*)

idf_component_register(
  SRCS ${uart_wrapper}
  INCLUDE_DIRS "include"
  REQUIRES driver result
)