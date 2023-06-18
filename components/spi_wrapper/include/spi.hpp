#pragma once

#include <memory.h>

#include <esp_log.h>
#include <driver/spi_master.h>
#include <result.hpp>

class SPIDevice {
  static constexpr const char* TAG = "SPIDevice";

  spi_device_handle_t device;

 public:
  SPIDevice(spi_host_device_t host, int ss) {
    spi_device_interface_config_t config = {.command_bits = 0,
                                            .address_bits = 0,
                                            .dummy_bits = 0,
                                            .mode = 0,
                                            .duty_cycle_pos = 128,
                                            .cs_ena_pretrans = 0,
                                            .cs_ena_posttrans = 0,
                                            .clock_speed_hz = 1024 * 1024 * 2,
                                            .spics_io_num = ss,

                                            .queue_size = 20};
    auto ret = spi_bus_add_device(host, &config, &this->device);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Device Added. (CS = %d)", ss);
    } else {
      ESP_LOGE(TAG, "Device Add failed: %s", esp_err_to_name(ret));
      abort();
    }
  }
  TaskResult Transfer(uint8_t* buf, size_t size, const char* name = "SPI") {
    spi_transaction_t a{.flags = 0,
                        .cmd = 0,
                        .addr = 0,
                        .length = 0,
                        .rxlength = 0,
                        .user = nullptr,
                        .tx_buffer = nullptr,
                        .rx_buffer = nullptr};

    a.tx_buffer = buf;
    a.length = size * 8;

    auto buffer = (void*)new char[size];
    memset(buffer, 0x77, size);
    a.rx_buffer = buffer;

    auto ret = spi_device_transmit(this->device, &a);
    if (ret != ESP_OK) return ret;

    // ret = spi_device_polling_transmit(this->device, &a);
    // if (ret != ESP_OK) return ret;

    if (name != nullptr) {
      if (size == a.rxlength / 8) {
        for (size_t i = 0; i < size; i++) {
          printf("%10s: %02X <=> %2X\n", name, buf[i], ((uint8_t*)buffer)[i]);
        }
      } else {
        printf("Sending: \n  %s", name);
        for (size_t i = 0; i < size; i++) {
          printf("%02X ", buf[i]);
          if (i % 16 == 15) printf("\n  %s", name);
        }
        printf("\n");
        printf("Received:\n  %s", name);
        for (size_t i = 0; i < a.rxlength / 8; i++) {
          printf("%02x ", buf[i]);
          if (i % 16 == 15) printf("\n  %s", name);
        }
        printf("\n");
      }
    }

    memcpy(buf, buffer, size);
    delete buffer;

    return TaskResult::Ok();
  }
  /*
    size_t GetRXBufferDataLength();

    void Flush();

    size_t Send(uint8_t* buf, size_t size);
    Result<ssize_t> Recv(uint8_t* buf, size_t size, TickType_t timeout);

    Result<uint8_t> RecvChar(TickType_t timeout = 1000 / portTICK_PERIOD_MS);
    void SendChar(uint8_t);

    TaskResult RecvExactly(uint8_t* buf, size_t size,
                           TickType_t timeout = portMAX_DELAY); */
};

class SPIMaster {
  static constexpr const char* TAG = "SPIMaster";

  spi_host_device_t spi_host;

 public:
  SPIMaster(spi_host_device_t spi_host, int mosi, int miso, int sclk)
      : spi_host(spi_host) {
    spi_bus_config_t config{
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
        .flags = SPICOMMON_BUSFLAG_MASTER,
    };
    auto ret = spi_bus_initialize(spi_host, &config, SPI_DMA_CH_AUTO);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "SPI Bus Initialized (MOSI = %d, MISO = %d, SCLK = %d)",
               mosi, miso, sclk);
    } else {
      ESP_LOGE(TAG, "SPI Bus Initialization Failed: %s", esp_err_to_name(ret));
      while (1)
        ;
    }
  }

  SPIDevice NewDevice(int ss) { return SPIDevice(this->spi_host, ss); }
};
