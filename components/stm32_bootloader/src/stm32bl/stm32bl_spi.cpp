#include <stm32bl/stm32bl_spi.hpp>

#include <helper.hpp>

#include <memory>
#include <ranges>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_debug_helpers.h>

namespace connection::application::stm32bl {
void Stm32BootLoaderSPI::RecvACK(TickType_t timeout) {
  auto trace_ = this->device.IsTraceEnabled();
  this->device.SetTraceEnabled(false);
  int fail_count = 0;

  this->device.SendChar(0x5A);
  this->device.RecvChar();

  while (true) {
    this->device.SendChar(0x00);
    if (auto ch = this->device.RecvChar(); ch == 0x79) {
      break;
    } else if (ch == 0x1f) {
      ESP_LOGW(TAG, "NACK");
      throw ACKFailed();
    } else {
      fail_count++;
      if (fail_count % 100 == 0) {
        // ESP_LOGW(TAG, "STM32 SPI ACK Fails %d times (wait 0.5 seconds)",
        //          fail_count);
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      if (fail_count % 100000 == 0) {
        throw BootLoaderNotBooted();
      }
    }
  }
  this->device.SendChar(0x79);
  this->device.RecvChar();
  this->device.SetTraceEnabled(trace_);
  return;
}

void Stm32BootLoaderSPI::Sync() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  this->device.SendChar(0x5A);

  if (auto ret = this->device.RecvChar(); ret != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret, 0xA5);
    throw ACKFailed();
  }
  this->RecvACK();

  ESP_LOGI(TAG, "Connection established");

  return;
}

void Stm32BootLoaderSPI::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{0x5A, cmd, (uint8_t)(cmd ^ 0xff)};

  this->device.Send(buf);
  this->device.Recv(buf);

  this->RecvACK();

  return;
}

void Stm32BootLoaderSPI::ReadData(std::vector<uint8_t> &buf) {
  this->device.SendChar(0x00);
  this->device.RecvChar();

  this->ReadDataWithoutHeader(buf);
}

void Stm32BootLoaderSPI::ReadDataWithoutHeader(std::vector<uint8_t> &buf) {
  std::ranges::fill(buf, 0x00);

  this->device.Send(buf);
  this->device.Recv(buf);
}

Stm32BootLoaderSPI::Stm32BootLoaderSPI(idf::GPIONum reset, idf::GPIONum boot0,
                                       idf::SPIMaster &spi_host, idf::CS cs)
    : STM32BootLoader(reset, boot0), device(spi_host, cs) {}

Stm32BootLoaderSPI::~Stm32BootLoaderSPI() = default;

void Stm32BootLoaderSPI::SendData(OutboundData &data) {
  using enum OutboundData::SizeMode;

  switch (data.size) {
    case kU8:
      assert(data.data.size() <= 0xff);
      this->device.SendChar((uint8_t)data.data.size());
      break;
    case kU16:
      assert(data.data.size() <= 0xffff);
      this->device.SendU16((uint16_t)data.data.size());
      break;

    default:  // includes kNone
      break;
  }

  this->device.Send(data.data);

  if (data.with_checksum) {
    this->device.SendChar(CalculateChecksum(data.data));
  }
}

void Stm32BootLoaderSPI::SendFlashPage(SpecialFlashPage page) {
  std::vector<uint8_t> buf(3);
  buf[0] = ((uint16_t)page >> 8);
  buf[1] = ((uint16_t)page & 0xff);
  buf[2] = buf[0] ^ buf[1];

  this->device.Send(buf);
  this->device.Recv(buf);
  this->RecvACK();
}

void Stm32BootLoaderSPI::SendFlashPage(std::vector<FlashPage> &pages) {
  Checksum checksum;
  {
    std::vector<uint8_t> buf(3);
    buf[0] = (uint8_t)(pages.size() >> 8);
    buf[1] = pages.size() & 0xff;
    checksum << (uint16_t)pages.size();
    buf[2] = (uint8_t)checksum;

    this->device.Send(buf);
    this->device.Recv(buf);
    this->RecvACK();
  }

  // Send pages and checksum
  {
    std::vector<uint8_t> buf(pages.size() * 2, 0x77);
    for (size_t i = 0; i < pages.size(); i++) {
      buf[2 * i] = pages[i] >> 8;
      buf[2 * i + 1] = pages[i] & 0xff;
    }
    checksum.Reset();
    checksum << (uint8_t)0x5a;  // WHAT IS 0x5A (NANNMO-WAKARAN)
    checksum << buf;
    buf.push_back((uint8_t)checksum);

    this->device.Send(buf);
    this->device.Recv(buf);
    this->RecvACK();
  }
}

void Stm32BootLoaderSPI::SendAddress(uint32_t address) {
  uint32_t ch = address;
  ch = (ch >> 16) ^ (ch & 0xffff);
  ch = (ch >> 8) ^ (ch & 0xff);

  this->device.SendU32(address);
  this->device.RecvU32();

  this->device.SendChar((char)ch);
  this->device.RecvChar();

  this->RecvACK();
}

void Stm32BootLoaderSPI::SendDataWithChecksum(std::vector<uint8_t> &data) {
  auto n = (uint8_t)(data.size() - 1);

  Checksum checksum;
  checksum << n;
  checksum << data;

  this->device.SendChar(n);
  this->device.RecvChar();

  this->device.Send(data);
  this->device.Recv(data);

  this->device.SendChar((uint8_t)checksum);
  this->device.RecvChar();

  this->RecvACK();
}

}  // namespace connection::application::stm32bl