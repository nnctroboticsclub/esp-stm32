# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/src/*.*)

# idf_component_register(SRCS ${app_sources})
# list(APPEND app_features_required_components mdns)
# list(APPEND app_features_required_components wifi_wrapper)
# list(APPEND app_features_required_components stm32_remote_controller)
# list(APPEND app_features_required_components driver nvs_flash esp_event console esp-idf-cxx esp_http_server )

idf_component_register(SRCS ${app_sources}
  INCLUDE_DIRS "include"

  REQUIRES ${app_features_required_components} stm32 nvs_flash wifi_wrapper stm32_remote_controller
)