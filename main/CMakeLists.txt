# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/src/*.*)

# idf_component_register(SRCS ${app_sources})
set(app_features_required_components)

if(CONFIG_MDNS_ENABLED)
  list(APPEND app_features_required_components mdns)
endif()

if(CONFIG_USE_NETWORK)
  list(APPEND app_features_required_components wifi_wrapper)
endif()

if(CONFIG_STM32_BOOTLOADER_DRIVER)
  list(APPEND app_features_required_components stm32_bootloader)
endif()

if(CONFIG_STM32_REMOTE_CONTROLLER_DRIVER)
  list(APPEND app_features_required_components stm32_remote_controller)
endif()

idf_component_register(SRCS ${app_sources}
  INCLUDE_DIRS "include"
  REQUIRES ${app_features_required_components} driver nvs_flash result esp_event)