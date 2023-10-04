# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(
  SRC_DIRS src
  INCLUDE_DIRS "include"

  REQUIRES data-proxy stm32 nvs_flash wifi_wrapper esp_http_server
)