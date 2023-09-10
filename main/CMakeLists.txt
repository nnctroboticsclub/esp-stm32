# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

idf_component_register(
  SRC_DIRS src
  INCLUDE_DIRS "include"

  REQUIRES ${app_features_required_components} stm32 data_proxy_master nvs_flash wifi_wrapper esp_http_server
)