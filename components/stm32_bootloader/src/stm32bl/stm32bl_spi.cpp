#include <stm32bl/stm32bl_spi.hpp>

#include <memory>
#include <ranges>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_debug_helpers.h>

namespace connection::application::stm32bl {
void Stm32BootLoaderSPI::WaitACKFrame() {
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
  this->device.SetTraceEnabled(true);
  return;
}

void Stm32BootLoaderSPI::Synchronization() {
  ESP_LOGI(TAG, "Sync...");

  std::vector<uint8_t> buf;

  this->device.SendChar(0x5A);

  if (auto ret = this->device.RecvChar(); ret != 0xA5) {
    ESP_LOGE(TAG, "Failed Sync (ACK Return value = %#02x != %#02x)", ret, 0xA5);
    throw ACKFailed();
  }
  this->WaitACKFrame();

  ESP_LOGI(TAG, "Connection established");

  return;
}

void Stm32BootLoaderSPI::CommandHeader(uint8_t cmd) {
  std::vector<uint8_t> buf{0x5A, cmd, (uint8_t)(cmd ^ 0xff)};
  this->device.Send(buf);

  this->device.Recv(buf);

  this->WaitACKFrame();

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

void Stm32BootLoaderSPI::Connect() {
  ESP_LOGI(TAG, "Connect...");

  while (true) {
    this->BootBootLoader();

    try {
      this->Synchronization();
    } catch (ACKFailed &) {
      ESP_LOGE(TAG, "Failed to connect to STM32 Bootloader");
      continue;
    }
    break;
  }

  this->Get();
}

void Stm32BootLoaderSPI::Get() {
  this->CommandHeader(0x00);

  std::vector<uint8_t> buf(2);
  this->ReadData(buf);
  auto n = buf[0];
  auto version = buf[1];

  ESP_LOGI(TAG, "Bootloader version: %d.%d", version >> 4, version & 0x0f);

  std::vector<uint8_t> raw_command(n);
  this->ReadDataWithoutHeader(raw_command);

  this->commands.get = raw_command[0];
  this->commands.get_version = raw_command[1];
  this->commands.get_id = raw_command[2];
  this->commands.read_memory = raw_command[3];
  this->commands.go = raw_command[4];
  this->commands.write_memory = raw_command[5];
  this->commands.erase = raw_command[6];
  this->commands.write_protect = raw_command[7];
  this->commands.write_unprotect = raw_command[8];
  this->commands.readout_protect = raw_command[9];
  this->commands.readout_unprotect = raw_command[10];
  if (n > 0x0c) {
    this->commands.get_checksum = raw_command[11];
  }

  this->WaitACKFrame();
  return;
}

void Stm32BootLoaderSPI::Erase(SpecialFlashPage page) {
  ESP_LOGI(TAG, "Erasing %s", SpecialFlashPageToString(page).c_str());
  this->CommandHeader(this->commands.erase);

  std::vector<uint8_t> buf(3);
  buf[0] = ((uint16_t)page >> 8);
  buf[1] = ((uint16_t)page & 0xff);
  buf[2] = buf[0] ^ buf[1];

  this->device.Send(buf);
  this->device.Recv(buf);
  this->WaitACKFrame();

  return;
}

void Stm32BootLoaderSPI::Erase(std::vector<FlashPage> &pages) {
  ESP_LOGI(TAG, "Erasing %d pages", pages.size());
  ESP_LOGI(TAG, "  %08x --> %08x", 0x0800'0000 + pages[0] * 0x800,
           0x0800'0000 + pages[pages.size() - 1] * 0x800);
  this->CommandHeader(this->commands.erase);

  uint8_t checksum = 0;

  // Send a pages
  {
    std::vector<uint8_t> buf(3);
    buf[0] = pages.size() >> 8;
    buf[1] = pages.size() & 0xff;
    checksum ^= CalculateChecksum(buf);
    buf[2] = checksum;

    this->device.Send(buf);
    this->device.Recv(buf);
    this->WaitACKFrame();
  }

  // Send pages and checksum
  {
    std::vector<uint8_t> buf(pages.size() * 2, 0x77);
    for (size_t i = 0; i < pages.size(); i++) {
      buf[2 * i] = pages[i] >> 8;
      buf[2 * i + 1] = pages[i] & 0xff;
    }
    checksum = 0x5a ^ CalculateChecksum(buf);  // WHAT 0x5A
    buf.push_back(checksum);

    this->device.Send(buf);
    this->device.Recv(buf);
    this->WaitACKFrame();
  }

  return;
}

void Stm32BootLoaderSPI::WriteMemoryBlock(uint32_t addr,
                                          std::vector<uint8_t> &buffer) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", addr, buffer.size());
  this->CommandHeader(this->commands.write_memory);

  {
    this->device.SendU32(addr);
    this->device.RecvU32();

    this->device.SendChar(0x00);
    this->device.RecvChar();

    // std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
    //                          (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
    // buf[4] = CalculateChecksum(buf);
    // this->device.Send(buf);
    // this->device.Recv(buf);

    this->WaitACKFrame();
  }

  {
    std::vector<uint8_t> buf{(uint8_t)(buffer.size() - 1)};

    this->device.Send(buf);
    this->device.Recv(buf);
    this->device.Send(buffer);
    this->device.Recv(buffer);

    buf[0] = CalculateChecksum(buffer) ^ buf[0];
    this->device.Send(buf);
    this->device.Recv(buf);

    this->WaitACKFrame();
  }

  return;
}

void Stm32BootLoaderSPI::Go(uint32_t addr) {
  ESP_LOGI(TAG, "Go to %08lx", addr);
  this->CommandHeader(this->commands.go);

  std::vector<uint8_t> buf{(uint8_t)(addr >> 24), (uint8_t)(addr >> 16),
                           (uint8_t)(addr >> 8), (uint8_t)(addr & 0xff)};
  buf[4] = CalculateChecksum(buf);
  this->device.Send(buf);
  this->device.Recv(buf);

  this->WaitACKFrame();

  return;
}

}  // namespace connection::application::stm32bl