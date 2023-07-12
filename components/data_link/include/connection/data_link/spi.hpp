#pragma once

#include <spi_host_cxx.hpp>

#include <string>
#include <stdexcept>
#include <vector>
#include <queue>

#include "connection/data_link/base.hpp"

namespace connection::data_link {

class SPIError : public std::runtime_error {
 public:
  explicit SPIError(const char *msg) : std::runtime_error(msg) {}
};

class SPIDevice : public RecvAndSend {
  static constexpr const char *TAG = "SPIDevice";
  std::shared_ptr<idf::SPIDevice> device;
  std::queue<uint8_t> queue;

  SPIDevice();

 public:
  explicit SPIDevice(idf::SPIMaster &master, idf::CS &cs);

  size_t Send(std::vector<uint8_t> &buf) override;

  size_t Recv(std::vector<uint8_t> &buf,
              TickType_t timeout = 1000 / portTICK_PERIOD_MS) override;
};
}  // namespace connection::data_link