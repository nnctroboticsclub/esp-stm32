#include <connection/data_link/spi.hpp>

#include <esp_log.h>

namespace connection::data_link {
using idf::CS;
using idf::SPIMaster;

SPIDevice::SPIDevice() = default;

SPIDevice::SPIDevice(SPIMaster &master, CS cs) : SPIDevice() {
  this->device = master.create_dev(cs, idf::Frequency::MHz(1));
}
SPIDevice::SPIDevice(std::shared_ptr<SPIMaster> master, CS cs) : SPIDevice() {
  this->device = master->create_dev(cs, idf::Frequency::MHz(1));
}

size_t SPIDevice::Send(const std::vector<uint8_t> &buf) {
  if (this->IsTraceEnabled())
    ESP_LOG_BUFFER_HEX("SPIDevice-->", buf.data(), buf.size());

  auto desc = this->device->transfer(buf);
  desc.wait();

  if (!desc.valid()) {
    throw SPIError("Failed to send data");
  }

  for (auto ch : desc.get()) {
    this->queue.push(ch);
  }

  return buf.size();
}

size_t SPIDevice::Recv(std::vector<uint8_t> &buf, TickType_t timeout) {
  if (this->queue.size() < buf.size()) {
    throw SPIError("No data to read");
  }

  for (size_t i = 0; i < buf.size(); i++) {
    buf[i] = this->queue.front();
    this->queue.pop();
  }
  if (this->IsTraceEnabled())
    ESP_LOG_BUFFER_HEX("SPIDevice<--", buf.data(), buf.size());

  return buf.size();
}
}  // namespace connection::data_link