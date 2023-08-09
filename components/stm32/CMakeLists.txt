# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE stm32_sources ${CMAKE_SOURCE_DIR}/components/stm32/src/*.*)

idf_component_register(
  SRCS ${stm32_sources}
  REQUIRES data_link driver esp-idf-cxx
  INCLUDE_DIRS "include"
)
