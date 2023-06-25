# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE wifi_wrapper_sources ${CMAKE_SOURCE_DIR}/components/wifi_wrapper/src/*.*)

idf_component_register(
  SRCS ${wifi_wrapper_sources}
  INCLUDE_DIRS "include"
  REQUIRES esp_wifi nvs_flash esp_netif wpa_supplicant
)