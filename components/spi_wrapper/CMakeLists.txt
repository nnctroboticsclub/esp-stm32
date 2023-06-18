# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE spi_wrapper ${CMAKE_SOURCE_DIR}/components/spi_wrapper/src/*.*)

idf_component_register(
  SRCS ${spi_wrapper}
  INCLUDE_DIRS "include"
  REQUIRES driver result
)