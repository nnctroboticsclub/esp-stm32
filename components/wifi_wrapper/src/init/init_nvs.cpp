#include "init.hpp"

#include <esp_log.h>
#include <nvs_flash.h>

void wifi::init::init_nvs() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  ESP_LOGI(TAG, "Initializing NVS");

  esp_err_t err;
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}