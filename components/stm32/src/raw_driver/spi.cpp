#include <memory>
#include <vector>
#include <stm32/raw_driver/impl/spi.hpp>

namespace stm32::raw_driver::impl {
using SPIDevice = connection::data_link::SPIDevice;

SPI::SPI(std::shared_ptr<SPIDevice> device) : device(device) {
  this->device->SetTraceEnabled(true);
}
SPI::SPI(std::shared_ptr<idf::SPIMaster> master, idf::CS chip_select)
    : SPI(std::make_shared<SPIDevice>(master, chip_select)) {}

SPI::~SPI() = default;

void SPI::ACK(TickType_t timeout) {
  auto trace_ = this->device->IsTraceEnabled();
  this->device->SetTraceEnabled(false);

  int fail_count = 0;

  this->device->SendChar(0x5A);
  this->device->RecvChar();

  while (true) {
    this->device->SendChar(0x00);

    if (auto res = this->device->RecvChar(timeout); res == 0x79) {
      break;
    } else if (res == 0x1f) {
      ESP_LOGW(TAG, "Explict NACK");
      throw ACKFailed();
    }

    fail_count++;
    if (fail_count % 10000 == 0) {
      ESP_LOGW(TAG, "ACK Fails %d times (wait 0.5 seconds)", fail_count);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    if (fail_count == 100000) {
      throw NoData();
    }
  }
  this->device->SendChar(0x79);
  this->device->RecvChar();
  this->device->SetTraceEnabled(trace_);
}

void SPI::Send(OutboundData const &data) {
  using enum OutboundData::SizeMode;
  using enum OutboundData::ChecksumMode;

  switch (data.size) {
    case kU8:
      assert(data.data.size() <= 0x100);
      this->device->SendChar(uint8_t(data.data.size() - 1));
      this->device->RecvChar();
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->device->SendU16((uint16_t)data.data.size());
      this->device->RecvChar();
      this->device->RecvChar();
      break;

    default:  // includes kNone
      break;
  }

  this->device->Send(data.data);

  std::vector<uint8_t> dummy(data.data.size());
  this->device->Recv(dummy);

  switch (data.checksum) {
    case kUnused:
      break;
    case kWithLength:
    case kData:
      uint8_t checksum = data.CalculateChecksum();
      this->device->SendChar(checksum);
      this->device->RecvChar();
      break;
  }

  if (data.no_ack) {
    ESP_LOGI(TAG, "No ACK");
  } else {
    this->ACK();
  }
}
std::vector<uint8_t> SPI::Recv(size_t length, bool resume) {
  std::vector<uint8_t> result(length, 0x00);

  if (!resume) {
    this->device->SendChar(0x00);
    this->device->RecvChar();
  }

  this->device->Send(result);
  this->device->Recv(result);

  return result;
}

void SPI::CommandHeader(uint8_t command) {
  std::vector<uint8_t> buf{0x5A, command, (uint8_t)(command ^ 0xff)};

  this->device->Send(buf);
  this->device->Recv(buf);

  this->ACK();
}

void SPI::Sync() {
  // ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  this->device->SendChar(0x5A);
  if (auto ret = this->device->RecvChar(); ret != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret, 0xA5);
    throw SyncFailed();
  }
  this->ACK();

  ESP_LOGI(TAG, "Connection established");
}

}  // namespace stm32::raw_driver::impl
