# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(
  SRCS src/uart.cpp src/connection/data_link/base.cpp
  INCLUDE_DIRS "include"
  REQUIRES driver
)