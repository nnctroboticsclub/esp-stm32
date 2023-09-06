#include <connection/data_link/i2c_dev.hpp>

#include <esp_log.h>

namespace connection::data_link {
using idf::I2CAddress;
using idf::I2CMaster;

I2CDevice::~I2CDevice() = default;

I2CDevice::I2CDevice(I2CMaster &master, I2CAddress address)
    : I2CDevice(std::shared_ptr<I2CMaster>(&master), address) {}
I2CDevice::I2CDevice(std::shared_ptr<I2CMaster> master, I2CAddress address)
    : master(master), device_address(address) {}

size_t I2CDevice::Send(const std::vector<uint8_t> &buf) {
  if (this->IsSendTraceEnabled())
    ESP_LOG_BUFFER_HEX("I2CDevice-->", buf.data(), buf.size());

  this->master->sync_write(this->device_address, buf);

  return buf.size();
}

size_t I2CDevice::Recv(std::vector<uint8_t> &buf, TickType_t timeout) {
  // received iterator is not valid after sync_read returns
  for (auto received =
           this->master->sync_read(this->device_address, buf.size());
       auto ch : received) {
    buf.push_back(ch);
  }

  if (this->IsRecvTraceEnabled())
    ESP_LOG_BUFFER_HEX("I2CDevice<--", buf.data(), buf.size());

  return buf.size();
}
}  // namespace connection::data_link