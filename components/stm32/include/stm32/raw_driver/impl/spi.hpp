#pragma once

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <memory>

#include <connection/data_link/spi.hpp>

#include "../types/error.hpp"
#include "./base.hpp"

namespace stm32::raw_driver::impl {
using SpiDataLink = connection::data_link::SPIDevice;

class SPI : public RawDriverBase {
  static constexpr const char *TAG = "[STM32-BL] RawDriver<SPI>";

  std::shared_ptr<SpiDataLink> device;

 public:
  explicit SPI(std::shared_ptr<connection::data_link::SPIDevice> device);

  ~SPI() override;

  void ACK(TickType_t timeout = portMAX_DELAY) override;

  void Send(OutboundData const &data) override;
  void Recv(InboundData &&data) override;

  void CommandHeader(uint8_t command) override;

  void Sync() override;
};
}  // namespace stm32::raw_driver::impl
