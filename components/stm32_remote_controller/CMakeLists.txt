# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE stm32_rc_sources ${CMAKE_SOURCE_DIR}/components/stm32_remote_controller/src/*.*)

idf_component_register(
  SRCS ${stm32_rc_sources}
  INCLUDE_DIRS "include"
  REQUIRES result uart_wrapper mutex
)