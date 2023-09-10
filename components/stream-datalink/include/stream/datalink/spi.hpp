#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <queue>

#include <spi_host_cxx.hpp>

#include <stream-base.hpp>

namespace stream::datalink {

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
  explicit SPIDevice(idf::SPIMaster &master, idf::CS cs);
  explicit SPIDevice(std::shared_ptr<idf::SPIMaster> master, idf::CS cs);

  size_t Send(const std::vector<uint8_t> &buf) override;

  size_t Recv(std::vector<uint8_t> &buf, DeltaTimeMs timeout = 1000) override;
};
}  // namespace stream