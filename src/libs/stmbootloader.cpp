#include "stmbootloader.hpp"

#include <esp_log.h>
#include <memory.h>

STMBootLoader::ACK STMBootLoader::RecvACK(TickType_t timeout) {
  auto ack = this->uart.RecvChar(timeout);

  if (ack == 0x79) return ACK::ACK;
  if (ack == 0x1f) return ACK::NACK;

  ESP_LOGE(TAG, "Failed to receive ACK");
  this->in_error_state = true;
  // while (1) vTaskDelay(100 / portTICK_PERIOD_MS);
  return ACK::NACK;
}

void STMBootLoader::SendWithChecksum(char* buf, size_t size) {
  uint8_t checksum = 0;
  for (int i = 0; i < size; i++) {
    checksum ^= buf[i];
  }
  this->uart.Send(buf, size);
  this->uart.SendChar(checksum);
}

void STMBootLoader::SendU16(uint8_t high, uint8_t low, bool with_checksum) {
  char buf[3];
  buf[0] = high;
  buf[1] = low;
  buf[2] = buf[0] ^ buf[1];
  this->uart.Send(buf, with_checksum ? 3 : 2);
}

void STMBootLoader::SendU16(uint16_t value, bool with_checksum) {
  this->SendU16((value >> 8) & 0xff, value & 0xff, with_checksum);
}

void STMBootLoader::SendCommand(uint8_t command) {
  char buf[2];
  buf[0] = command;
  buf[1] = buf[0] ^ 0xff;
  this->uart.Send(buf, 2);
}

void STMBootLoader::SendAddress(uint32_t address) {
  char buf[5];
  buf[0] = (address >> 24) & 0xff;
  buf[1] = (address >> 16) & 0xff;
  buf[2] = (address >> 8) & 0xff;
  buf[3] = (address >> 0) & 0xff;
  buf[4] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
  this->uart.Send(buf, 5);
}

void STMBootLoader::DoGetVersion() {
  this->SendCommand(this->get_version);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Get Version command");
    this->in_error_state = true;
    return;
  }

  char buf[3];
  this->uart.Recv(buf, 3, 500 / portTICK_PERIOD_MS);
  this->version.major = buf[0] >> 4;
  this->version.major = buf[0] >> 4;
  this->version.option1 = buf[1];
  this->version.option1 = buf[2];
  this->RecvACK();
}

void STMBootLoader::DoExtendedErase(FlashPage page) {
  this->SendCommand(this->erase);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Extended Erase command (command)");
    this->in_error_state = true;
    return;
  }

  this->SendU16(page, true);
  if (this->RecvACK(10000 / portTICK_PERIOD_MS) == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Extended Erase command (bank)");
    this->in_error_state = true;
    return;
  }
}

void STMBootLoader::DoExtendedErase(std::vector<uint16_t> pages) {
  this->SendCommand(this->erase);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Extended Erase command (command)");
    this->in_error_state = true;
    return;
  }

  char* buffer = new char[pages.size() * 2 + 2];
  buffer[0] = (pages.size() - 1) >> 8;
  buffer[1] = (pages.size() - 1) & 0xff;
  for (int i = 0; i < pages.size(); i++) {
    buffer[2 + i * 2] = (pages[i] >> 8) & 0xff;
    buffer[2 + i * 2 + 1] = pages[i] & 0xff;
  }

  this->SendWithChecksum(buffer, pages.size() * 2 + 2);

  delete buffer;

  if (this->RecvACK(10000 / portTICK_PERIOD_MS) == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Extended Erase command (bank)");
    this->in_error_state = true;
    return;
  }
}

void STMBootLoader::DoErase(FlashPage page) {
  // TODO(syoch): Impl
}
void STMBootLoader::DoErase(std::vector<uint16_t> pages) {
  // TODO(syoch): Impl
}

STMBootLoader::STMBootLoader(gpio_num_t reset, gpio_num_t boot0,
                             uart_port_t num, int tx, int rx)
    : uart(num, tx, rx, 112500), reset(reset), boot0(boot0) {
  gpio_set_direction(reset, GPIO_MODE_OUTPUT);
  gpio_set_direction(boot0, GPIO_MODE_OUTPUT);

  gpio_set_level(reset, 1);
}

STMBootLoader::Version* STMBootLoader::GetVersion() {
  if (!this->version.is_valid) {
    this->version.is_valid = true;
    this->GetVersion();
  }

  return &this->version;
}

void STMBootLoader::BootBootLoader() {
  ESP_LOGI(TAG, "Booting BootLoader");
  this->in_error_state = false;

  ESP_LOGI(TAG, "- reset = 0");
  gpio_set_level(this->reset, 0);
  vTaskDelay(200 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "- boot0 = 1");
  gpio_set_level(this->boot0, 1);
  vTaskDelay(200 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "- reset = 1");
  gpio_set_level(this->reset, 1);
  vTaskDelay(200 / portTICK_PERIOD_MS);

  ESP_LOGI(TAG, "- boot0 = 0");
  gpio_set_level(this->boot0, 0);

  this->uart.Flush();
}

void STMBootLoader::Sync() {
  this->uart.Send("\x7f", 1);
  auto ret = this->uart.RecvChar();

  if (ret == 0x79) {
    ESP_LOGI(TAG, "Synced");
    return;
  } else if (ret == 0x1f) {
    ESP_LOGE(TAG, "Failed to sync (NACK)");
    this->in_error_state = true;
    return;
  } else if (ret == -1) {
    ESP_LOGE(TAG, "Failed to sync (timeout)");
    this->in_error_state = true;
    return;
  } else {
    ESP_LOGW(TAG, "Failed to sync (unknown %#02x)", ret);
    return;
  }
}

void STMBootLoader::Get() {
  this->SendCommand(this->get);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Get command");
    this->in_error_state = true;
    return;
  }

  this->ack = this->uart.RecvChar();
  auto n = this->uart.RecvChar();

  auto byte = this->uart.RecvChar();
  this->version.major = byte >> 4;
  this->version.minor = byte & 0xf;

  char buf[16];
  this->uart.Recv(buf, n, 100 / portTICK_PERIOD_MS);

  this->get = buf[0];
  this->get_version = buf[1];
  this->get_id = buf[2];
  this->read_memory = buf[3];
  this->go = buf[4];
  this->write_memory = buf[5];
  this->erase = buf[6];
  this->write_protect = buf[7];
  this->write_unprotect = buf[8];
  this->readout_protect = buf[9];
  this->readout_unprotect = buf[10];
  // TODO: Add GetChecksum command
  // this->get_checksum = buf[11];

  this->RecvACK();

  if (this->erase == 0x44) {
    this->use_extended_erase = true;
  }
}

int STMBootLoader::WriteMemoryBlock(uint32_t address, uint8_t* buffer,
                                    size_t size) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", address, size);
  this->uart.Send("\x31\xce", 2);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Write Memory command (command byte)");
    this->in_error_state = true;
    return -1;
  }

  this->SendAddress(address);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Write Memory command (address)");
    this->in_error_state = true;
    return -2;
  }

  this->uart.SendChar(size - 1);

  uint8_t checksum = size - 1;
  for (int i = 0; i < size; i++) {
    checksum ^= buffer[i];
  }
  this->uart.Send((char*)buffer, size);
  this->uart.SendChar(checksum);

  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Write Memory command(data bytes)");
    this->in_error_state = true;
    ESP_LOGE(TAG, "  - chunksum: %02x", checksum);
    this->in_error_state = true;
    return -3;
  }
  return size;
}

int STMBootLoader::WriteMemory(uint32_t address, unsigned char* buffer,
                               size_t size) {
  int remains = size;
  uint8_t* ptr = buffer;
  uint8_t buf[256];
  int offset = 0;
  while (remains > 0) {
    memset(buf, 0, 256);
    memcpy(buf, ptr, remains > 256 ? 256 : remains);
    this->WriteMemoryBlock(address + offset, buf, 256);
    remains = remains - 256 > 0 ? remains - 256 : 0;
    ptr += 256;
    offset += 256;
  }
  return size;
}

void STMBootLoader::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go command");
  this->uart.Send("\x21\xde", 2);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Go command (command byte)");
    this->in_error_state = true;
    return;
  }

  this->SendAddress(address);
  if (this->RecvACK() == STMBootLoader::ACK::NACK) {
    ESP_LOGE(TAG, "Failed to Go command (address)");
    this->in_error_state = true;
    return;
  }
}

void STMBootLoader::Erase(FlashPage page) {
  if (this->use_extended_erase) {
    this->DoExtendedErase(page);
  } else {
    this->DoErase(page);
  }
}

void STMBootLoader::Erase(std::vector<uint16_t> pages) {
  if (this->use_extended_erase) {
    this->DoExtendedErase(pages);
  } else {
    this->DoErase(pages);
  }
}

void STMBootLoader::BulkErase(std::vector<uint16_t> pages) {
  int all_pages = pages.size();

  std::vector<uint16_t> sub_pages;
  int i = 0;
  for (i = 0; i + 4 < pages.size(); i += 4) {
    sub_pages.clear();
    for (int j = 0; j < 4; j++) {
      sub_pages.push_back(pages[i + j]);
    }
    ESP_LOGI(TAG, "Erasing pages... (%d to %d pages, remains %d page)", i,
             i + 4, all_pages - i - 4);
    this->Erase(sub_pages);
  }

  if (all_pages % 4 != 0) {
    std::vector<uint16_t> sub_pages;
    for (int j = 0; j < pages.size() - i; j++) {
      sub_pages.push_back(pages[i + j]);
    }
    ESP_LOGI(TAG, "Erasing pages... (All %d pages)", all_pages);
    this->Erase(sub_pages);
  }
}

void STMBootLoader::Erase(uint32_t address, uint32_t length) {
  if (address + length > 0x0804'0000) {
    // bulk erase bank1
    this->Erase(STMBootLoader::bank1);

    // erase address + length - 0x0804'0000
    int bank2_erase_start = (address - 0x0804'0000) >> 11;
    int bank2_erase_end = (address + length - 0x0804'0000) >> 11;

    std::vector<uint16_t> pages;
    for (int i = bank2_erase_start; i < bank2_erase_end; i++) {
      pages.push_back(i);
    }
    this->BulkErase(pages);
  } else {
    int bank1_erase_start = (address - 0x0800'0000) >> 11;
    int bank1_erase_end = (address + length - 0x08'000000) >> 11;

    std::vector<uint16_t> pages;
    for (int i = bank1_erase_start; i < bank1_erase_end; i++) {
      pages.push_back(i);
    }
    this->BulkErase(pages);
  }
}