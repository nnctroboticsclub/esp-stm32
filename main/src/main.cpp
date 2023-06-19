#include <esp_log.h>

#include "config/config.hpp"
#include "init/init.hpp"
#include "bin.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs.h>
#include <esp_system.h>

#include "spi.hpp"
#include "libs/gpio.hpp"
const char* TAG = "Main";

#include <stm32bl/stm32bl_spi.hpp>

class UserButton {
  static constexpr const char* TAG = "UserButton";

 private:
  gpio_num_t pin;

 public:
  UserButton(gpio_num_t button) : pin(button) {
    ESP_LOGI(TAG, "Setting GPIO %d as input", button);
    gpio_set_direction(button, GPIO_MODE_INPUT);
  }

  void WaitUntilPressed() {
    while (gpio_get_level(this->pin) == 0) vTaskDelay(50 / portTICK_PERIOD_MS);

    while (gpio_get_level(this->pin) == 1) vTaskDelay(50 / portTICK_PERIOD_MS);
  }
};

void BootStrap() {
  SPIMaster master(VSPI_HOST, 23, 19, 18);
  {
    auto stm32bl_ns = new nvs::Namespace("a_s32bl");
    auto stm32bl = profile::SpiSTM32BootLoaderProfile(stm32bl_ns);

    stm32bl.reset = GPIO_NUM_27;
    stm32bl.boot0 = GPIO_NUM_26;
    stm32bl.cs = GPIO_NUM_5;
    stm32bl.spi_port = 1;

    stm32bl.Save();
  }

  {
    auto config = config::Config::GetInstance();
    config->server_profile.ip = (types::Ipv4){.ip_bytes = {192, 168, 11, 7}};
    config->server_profile.port = 8080;
    config->network_profiles[0].mode = types::NetworkMode::STA;
    config->network_profiles[0].name = "Network@Ryo";
    config->network_profiles[0].ssid = "***REMOVED***";
    config->network_profiles[0].password = "***REMOVED***";
    config->network_profiles[0].hostname = "esp32-0610";
    config->network_profiles[0].ip_mode = types::IPMode::DHCP;
    config->network_profiles[0].ip = 0;
    config->network_profiles[0].subnet = 0;
    config->network_profiles[0].gateway = 0;
    config->active_network_profile = 0;
  }

  init::init_data_server();

  xTaskCreate((TaskFunction_t)([](void* args) {
                while (1) {
                  vTaskDelay(50 / portTICK_PERIOD_MS);
                  ((DebuggerMaster*)args)->Idle();
                }
                return;
              }),
              "Debugger Idling Thread", 0x1000,
              config::Config::GetInstance()
                  ->stm32_remote_controller_profile.GetDebuggerMaster(),
              1, nullptr);
}

TaskResult Main() {
  ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  config::server.StartClientLoopAtForeground();

  // stm32bl::Stm32BootLoaderSPI bl(GPIO_NUM_27, GPIO_NUM_26, master, 5);
  //
  // RUN_TASK_V(bl.Connect());
  // RUN_TASK_V(bl.Get());
  //
  // bl.Erase(0x08000000, new_flash_len);
  // RUN_TASK_V(bl.WriteMemory(0x08000000, new_flash, new_flash_len));

  /*
  nvs_iterator_t it;
  auto ret = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL,
                            nvs_type_t::NVS_TYPE_ANY, &it);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Error ocurred while finding nvs entries: %s",
             esp_err_to_name(ret));

    return;
  }

  ESP_LOGI(TAG, "First it = %p", it);
  while (it) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);

    nvs_handle_t handle;
    nvs_open(info.namespace_name, NVS_READONLY, &handle);
    switch (info.type) {
      case NVS_TYPE_U8: {
        uint8_t value;
        nvs_get_u8(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (u8)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U16: {
        uint16_t value;
        nvs_get_u16(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (u16)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U32: {
        uint32_t value;
        nvs_get_u32(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %lu (u32)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_U64: {
        uint64_t value;
        nvs_get_u64(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %llu (u64)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I8: {
        int8_t value;
        nvs_get_i8(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (s8)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I16: {
        int16_t value;
        nvs_get_i16(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %d (s16)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I32: {
        int32_t value;
        nvs_get_i32(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %ld (s32)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_I64: {
        int64_t value;
        nvs_get_i64(handle, info.key, &value);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %lld (s64)", info.namespace_name,
                 info.key, value);
        break;
      }
      case NVS_TYPE_STR: {
        size_t length = 0;
        nvs_get_str(handle, info.key, NULL, &length);

        char* value = (char*)malloc(length);
        nvs_get_str(handle, info.key, value, &length);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = %s (str)", info.namespace_name,
                 info.key, value);

        free(value);
        break;
      }

      case NVS_TYPE_BLOB: {
        size_t length = 0;
        nvs_get_blob(handle, info.key, NULL, &length);

        uint8_t* value = (uint8_t*)malloc(length);
        nvs_get_blob(handle, info.key, (void*)value, &length);

        ESP_LOGI(TAG, "NVS Entry: %s::%s = <blob>", info.namespace_name,
                 info.key);

        printf("  ");
        for (size_t i = 0; i < length; i++) {
          printf("%02x ", value[i]);
          if (i % 16 == 15) {
            printf("\n  ");
          }
        }

        free(value);
        break;
      }
      case NVS_TYPE_ANY:
      default:
        ESP_LOGW(TAG, "NVS Entry: %s::%s = <unknown>", info.namespace_name,
                 info.key);
        break;
    }
    nvs_entry_next(&it);
    nvs_close(handle);
  }
  */

  return TaskResult::Ok();
}

extern "C" int app_main() {
  BootStrap();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}