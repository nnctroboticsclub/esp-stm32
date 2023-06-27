# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE stm32_bl_sources ${CMAKE_SOURCE_DIR}/components/stm32_bootloader/src/*.*)

idf_component_register(
  SRCS ${stm32_bl_sources}
  REQUIRES result uart_wrapper driver esp-idf-cxx
  INCLUDE_DIRS "include"
)