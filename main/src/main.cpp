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
  // auto config = config::Config::GetInstance();
  // config->server_profile.ip = (types::Ipv4){.ip_bytes = {192, 168, 0, 1}};
  // config->server_profile.port = 8080;
  // config->network_profiles[0].mode = types::NetworkMode::STA;
  // config->network_profiles[0].ip_mode = types::IPMode::DHCP;
  // config->network_profiles[0].name = "Network@home";
  // config->network_profiles[0].ssid = "";
  // config->network_profiles[0].password = "";
  // config->network_profiles[0].hostname = "esp32-0610";
  // config->network_profiles[0].ip = 0;
  // config->network_profiles[0].subnet = 0;
  // config->network_profiles[0].gateway = 0;
  // config->active_network_profile = 0;
  // config->stm32_bootloader_profile.reset = GPIO_NUM_19;
  // config->stm32_bootloader_profile.boot0 = GPIO_NUM_21;
  // config->stm32_bootloader_profile.uart_port = 1;
  // config->stm32_bootloader_profile.uart_tx = GPIO_NUM_17;  // TX2
  // config->stm32_bootloader_profile.uart_rx = GPIO_NUM_16;  // RX2

  // init::init_data_server();

  // xTaskCreate((TaskFunction_t)([](void* args) {
  //             while (1) {
  //               vTaskDelay(50 / portTICK_PERIOD_MS);
  //               ((DebuggerMaster*)args)->Idle();
  //             }
  //             return;
  //           }),
  //           "Debugger Idling Thread", 0x1000, &config::debugger, 1,
  //           nullptr);
}

class SPI_STM32BL {
  static constexpr const char* TAG = "STM32 BootLoader[SPI]";

 public:  // Debug public.
  SPIDevice device;

 private:
  enum ACK : uint8_t {
    ACK = 0x79,
    NACK = 0x1f,
  };
  struct {
    uint8_t get;
    uint8_t get_version;
    uint8_t get_id;
    uint8_t read_memory;
    uint8_t go;
    uint8_t write_memory;
    uint8_t special;
    uint8_t extended_special;
    uint8_t write_protect;
    uint8_t write_unprotect;
    uint8_t readout_protect;
    uint8_t readout_unprotect;
    uint8_t get_checksum;
  } commands;

  gpio_num_t reset, boot0;
  void DoSTM32Reset() {
    ESP_LOGI(TAG, "Booting Bootloader");
    gpio_set_level(this->boot0, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(this->reset, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(this->reset, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(this->boot0, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  TaskResult WaitACKFrame() {
    int fail_count = 0;
    uint8_t buf1 = 0x00;
    RUN_TASK_V(device.Transfer(&buf1, 1))
    while (1) {
      uint8_t buf1 = 0x00;
      RUN_TASK_V(device.Transfer(&buf1, 1))

      if (buf1 == 0x79) {
        break;
      } else if (buf1 == 0x1f) {
        ESP_LOGW(TAG, "NACK");
        break;
      } else {
        fail_count++;
        if (fail_count % 100 == 0) {
          ESP_LOGW(TAG, "STM32 SPI ACK Fails %d times", fail_count);
        }
        if (fail_count % 1000 == 0) {
          return ESP_ERR_INVALID_STATE;
        }
      }
    }
    buf1 = 0x79;
    RUN_TASK_V(device.Transfer(&buf1, 1));

    return TaskResult::Ok();
  }

  TaskResult Synchronization() {
    ESP_LOGI(TAG, "Sync...");

    uint8_t buf[2] = {0x5A};

    RUN_TASK_V(device.Transfer(buf, 1))
    if (buf[0] != 0xA5) {
      ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", buf[0],
               0xA5);
      return ESP_ERR_INVALID_STATE;
    }
    this->WaitACKFrame();

    ESP_LOGI(TAG, "Connection established");

    return TaskResult::Ok();
  }

 public:
  SPI_STM32BL(gpio_num_t reset, gpio_num_t boot0, SPIMaster& spi_master, int cs)
      : device(spi_master.NewDevice(cs)), reset(reset), boot0(boot0) {
    gpio_set_direction(this->reset, GPIO_MODE_OUTPUT);
    gpio_set_direction(this->boot0, GPIO_MODE_OUTPUT);
    gpio_set_level(this->reset, 1);
    gpio_set_level(this->boot0, 0);
  }

  TaskResult Connect() {
    ESP_LOGI(TAG, "Connect...");

    TaskResult ret = ESP_ERR_INVALID_STATE;
    while (ret.IsErr()) {
      this->DoSTM32Reset();
      ret = this->Synchronization();
    }

    {
      RUN_TASK_V(this->CommandHeader(0x01));

      uint8_t buf1 = 0x00;
      RUN_TASK_V(this->ReadData(&buf1, 1));

      RUN_TASK_V(this->WaitACKFrame());

      // uint8_t* buf2 = new uint8_t[buf[1]];
      // memset(buf2, 0x00, buf[1]);
      // buf2[2] = 0x79;
      // RUN_TASK_V(this->device.Transfer(buf2, buf[1]));
      //
      // this->commands.get = buf2[0];
      // this->commands.get_version = buf2[1];
      // this->commands.get_id = buf2[2];
      // this->commands.read_memory = buf2[3];
      // this->commands.go = buf2[4];
      // this->commands.write_memory = buf2[5];
      // this->commands.write_protect = buf2[6];
      // this->commands.write_unprotect = buf2[7];
      // this->commands.readout_protect = buf2[8];
      // this->commands.readout_unprotect = buf2[9];
      // if (buf[1] >= 0x0c) {
      //   this->commands.get_checksum = buf2[10];
      // }
    }

    return TaskResult::Ok();
  }

  TaskResult CommandHeader(uint8_t cmd) {
    uint8_t buf[] = {0x5A, cmd, (uint8_t)(cmd ^ 0xff)};

    RUN_TASK_V(this->device.Transfer(buf, 1));
    RUN_TASK_V(this->device.Transfer(buf + 1, 1));
    RUN_TASK_V(this->device.Transfer(buf + 2, 1));

    if (buf[2] != 0x79) {
      ESP_LOGE(TAG, "Command Header - buf[2] != 0x79 (actually: %#02x)",
               buf[2]);
      // return ESP_ERR_INVALID_RESPONSE;
    }

    RUN_TASK_V(this->WaitACKFrame());

    return TaskResult::Ok();
  }

  TaskResult ReadData(uint8_t* buf, size_t size) {
    uint8_t dummy = 0;
    RUN_TASK_V(this->device.Transfer(&dummy, 1));

    memset(buf, 0x77, size);

    RUN_TASK_V(this->device.Transfer(buf, size));

    return TaskResult::Ok();
  }
};

TaskResult Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  SPIMaster master(VSPI_HOST, 23, 19, 18);

  SPI_STM32BL bl(GPIO_NUM_27, GPIO_NUM_26, master, 5);

  bl.Connect();

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