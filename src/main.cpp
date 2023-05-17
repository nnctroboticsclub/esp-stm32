#include <esp_log.h>

#include "libs/wifi.hpp"
#include "./config.hpp"
#include "init/init.hpp"
#include <driver/uart.h>
#include "uart.hpp"
#include "bin.h"

using namespace app;

#define USER_PROGRAM_START 0x08000000

class STMBootLoader {
  static constexpr const char* TAG = "STMBootLoader";

 public:
  enum class ACK { ACK, NACK };
  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t ack = 0x79;

  struct {
    uint8_t major;
    uint8_t minor;
    uint8_t option1;
    uint8_t option2;
  } version;

  UART uart;
  gpio_num_t reset, boot0;

  STMBootLoader(gpio_num_t reset, gpio_num_t boot0, uart_port_t num)
      : uart(num, 9600), reset(reset), boot0(boot0) {
    gpio_set_direction(reset, GPIO_MODE_OUTPUT);
    gpio_set_direction(boot0, GPIO_MODE_OUTPUT);
  }

  void BootBootLoader() {
    ESP_LOGI(TAG, "Booting BootLoader");

    ESP_LOGI(TAG, "- reset = 0");
    gpio_set_level(this->reset, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "- boot0 = 1");
    gpio_set_level(this->boot0, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "- reset = 1");
    gpio_set_level(this->reset, 1);
    vTaskDelay(200 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "- boot0 = 0");
    gpio_set_level(this->boot0, 0);
  }

  ACK RecvACK() {
    auto ack = this->uart.RecvChar();

    if (ack == 0x79) return ACK::ACK;
    if (ack == 0x1f) return ACK::NACK;

    ESP_LOGE(TAG, "Failed to receive ACK");
    // while (1) vTaskDelay(100 / portTICK_PERIOD_MS);
    return ACK::NACK;
  }

  void SendAddress(uint32_t address) {
    char buf[5];
    buf[0] = (address >> 0) & 0xff;
    buf[1] = (address >> 8) & 0xff;
    buf[2] = (address >> 16) & 0xff;
    buf[3] = (address >> 24) & 0xff;
    buf[4] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
    this->uart.Send(buf, 5);
  }

  void Sync() {
    this->uart.Send("\x7f", 1);
    this->uart.RecvChar();
  }

  void Get() {
    this->uart.Send("\x00\xff", 2);
    if (this->RecvACK() == STMBootLoader::ACK::NACK) {
      ESP_LOGE(TAG, "Failed to Get command");
      return;
    }

    this->ack = this->uart.RecvChar();
    auto n = this->uart.RecvChar();

    auto byte = this->uart.RecvChar();
    this->version.major = byte >> 4;
    this->version.minor = byte & 0xf;

    char buf[16];
    this->uart.Recv(buf, n, 100 / portTICK_PERIOD_MS);

    this->ack = buf[0];
    this->get = buf[1];
    this->get_version = buf[2];
    this->get_id = buf[3];
    this->read_memory = buf[4];
    this->go = buf[5];
    this->write_memory = buf[6];
    this->erase = buf[7];
    this->write_protect = buf[8];
    this->write_unprotect = buf[9];
    this->readout_protect = buf[10];
    this->readout_unprotect = buf[11];

    this->RecvACK();
  }

  void GetVersion() {
    this->uart.Send("\x01\xfe", 2);
    if (this->RecvACK() == STMBootLoader::ACK::NACK) {
      ESP_LOGE(TAG, "Failed to Get Version command");
      return;
    }

    char buf[3];
    this->uart.Recv(buf, 3, 500 / portTICK_PERIOD_MS);
    this->version.major = buf[0] >> 4;
    this->version.major = buf[0] >> 4;
    this->version.option1 = buf[1];
    this->version.option1 = buf[2];
    this->RecvACK();
  }

  int WriteMemory(uint32_t address, uint8_t* buffer, size_t size) {
    ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", address, size);
    this->uart.Send("\x31\xce", 2);
    if (this->RecvACK() == STMBootLoader::ACK::NACK) {
      ESP_LOGE(TAG, "Failed to Write Memory command (command byte)");
      return -1;
    }

    this->SendAddress(address);
    if (this->RecvACK() == STMBootLoader::ACK::NACK) {
      ESP_LOGE(TAG, "Failed to Write Memory command (address)");
      return -2;
    }

    this->uart.SendChar(size - 1);

    uint8_t checksum = size - 1;
    for (int i = 0; i < size; i++) {
      checksum ^= buffer[i];
    }
    this->uart.Send((char*)buffer, size);
    this->uart.SendChar(checksum);

    if (this->RecvACK() == STMBootLoader::ACK::NACK) {
      ESP_LOGE(TAG, "Failed to Write Memory command(data bytes)");
      ESP_LOGE(TAG, "  - chunksum: %02x", checksum);
      return -3;
    }
    return size;
  }
};

const char* TAG = "Main";

void BootStrap() {
  init::init_serial();
  // init::init_mdns();
  // init::init_data_server();
}

void Main() {
  // ESP_LOGI(TAG, "Entering the Server's ClientLoop");
  // config::server.StartClientLoopAtForeground();

  STMBootLoader loader(RESET, BOOT0, UART_NUM_1);
  loader.BootBootLoader();
  loader.uart.Flush();
  vTaskDelay(200 / portTICK_PERIOD_MS);

  loader.Sync();
  loader.Get();
  loader.GetVersion();

  ESP_LOGI(TAG, "Boot Loader version = %d.%d", loader.version.major,
           loader.version.minor);

  loader.WriteMemory(USER_PROGRAM_START, new_flash, 0x100);

  for (int i = 0; i < 20;) {
    vTaskDelay(10 / portTICK_PERIOD_MS);

    auto length = loader.uart.GetRXBufferDataLength();
    if (length == 0) continue;
    ESP_LOGI(TAG, "length = %d, (i = %d)", length, i);

    char buf[1024];
    while (length > 0) {
      size_t chunk = loader.uart.Recv(buf, length > 1024 ? 1024 : length,
                                      1000 / portTICK_PERIOD_MS);
      length -= chunk;
    }
    loader.uart.Flush();
    i++;
  }

  return;

  int remain = new_flash_len;
  uint8_t* ptr = new_flash;
  for (int chunk = 0; chunk < new_flash_len / 256; chunk++) {
    auto ret = loader.WriteMemory(USER_PROGRAM_START + chunk * 0x100,  //
                                  ptr, remain > 256 ? 256 : remain);
    if (ret < 0) break;
    remain -= 0x100;
    ptr += 0x100;
  }
}

extern "C" void app_main() {
  BootStrap();
  Main();
  printf("Entering the idle loop\n");
  while (1) {
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}