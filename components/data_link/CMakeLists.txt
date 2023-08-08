# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(
  SRCS src/data_link/uart.cpp src/data_link/spi.cpp src/data_link/base.cpp
  INCLUDE_DIRS "include"
  REQUIRES driver esp-idf-cxx
)