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

  REQUIRES ${app_features_required_components} driver nvs_flash esp_event console esp-idf-cxx esp_http_server stm32
)