#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <queue>

#include <i2c_cxx.hpp>

#include <stream-base.hpp>

namespace stream::datalink {

class I2CError : public std::runtime_error {
 public:
  explicit I2CError(const char *msg) : std::runtime_error(msg) {}
};

class I2CDevice : public RecvAndSend {
  static constexpr const char *TAG = "I2CDevice";
  std::shared_ptr<idf::I2CMaster> master;
  idf::I2CAddress device_address;

 public:
  ~I2CDevice() override;

  explicit I2CDevice(idf::I2CMaster &master, idf::I2CAddress address);
  explicit I2CDevice(std::shared_ptr<idf::I2CMaster> master,
                     idf::I2CAddress address);

  size_t Send(const std::vector<uint8_t> &buf) override;

  size_t Recv(std::vector<uint8_t> &buf, DeltaTimeMs timeout = 1000) override;
};
}  // namespace stream