# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/src/*.*)

# idf_component_register(SRCS ${app_sources})
set(app_features_required_components)

list(APPEND app_features_required_components mdns)
list(APPEND app_features_required_components wifi_wrapper)
list(APPEND app_features_required_components stm32_bootloader)
list(APPEND app_features_required_components stm32_remote_controller)

idf_component_register(SRCS ${app_sources}
  INCLUDE_DIRS "include"
  REQUIRES ${app_features_required_components} driver nvs_flash result esp_event spi_wrapper)