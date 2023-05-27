#include "stmbootloader.hpp"

#include <esp_log.h>
#include <memory.h>

TaskResult STMBootLoader::RecvACK(TickType_t timeout) {
  RUN_TASK(this->uart.RecvChar(timeout), ack);

  if (ack == 0x79) return TaskResult::Ok();
  if (ack == 0x1f) {
    this->in_error_state = true;
    ESP_LOGE(TAG, "Received NACK");
    return ESP_ERR_INVALID_STATE;
  }

  ESP_LOGE(TAG, "Failed to receive ACK");
  this->in_error_state = true;
  return ESP_ERR_INVALID_RESPONSE;
}

void STMBootLoader::SendWithChecksum(uint8_t* buf, size_t size) {
  uint8_t checksum = 0;
  for (int i = 0; i < size; i++) {
    checksum ^= buf[i];
  }
  this->uart.Send(buf, size);
  this->uart.SendChar(checksum);
}

void STMBootLoader::SendU16(uint8_t high, uint8_t low, bool with_checksum) {
  uint8_t buf[3];
  buf[0] = high;
  buf[1] = low;
  buf[2] = buf[0] ^ buf[1];
  this->uart.Send(buf, with_checksum ? 3 : 2);
}

void STMBootLoader::SendU16(uint16_t value, bool with_checksum) {
  this->SendU16((value >> 8) & 0xff, value & 0xff, with_checksum);
}

void STMBootLoader::SendCommand(uint8_t command) {
  uint8_t buf[2];
  buf[0] = command;
  buf[1] = buf[0] ^ 0xff;
  this->uart.Send(buf, 2);
}

void STMBootLoader::SendAddress(uint32_t address) {
  uint8_t buf[5];
  buf[0] = (address >> 0x1c) & 0xff;
  buf[1] = (address >> 0x10) & 0xff;
  buf[2] = (address >> 0x08) & 0xff;
  buf[3] = (address >> 0x00) & 0xff;
  buf[4] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
  this->uart.Send(buf, 5);
}

TaskResult STMBootLoader::DoGetVersion() {
  this->SendCommand(this->get_version);
  RUN_TASK_V(this->RecvACK());

  uint8_t buf[3];
  this->uart.Recv(buf, 3, 500 / portTICK_PERIOD_MS);
  this->version.major = buf[0] >> 4;
  this->version.major = buf[0] >> 4;
  this->version.option1 = buf[1];
  this->version.option1 = buf[2];
  RUN_TASK_V(this->RecvACK());

  return TaskResult::Ok();
}

TaskResult STMBootLoader::DoExtendedErase(FlashPage page) {
  this->SendCommand(this->erase);
  RUN_TASK_V(this->RecvACK());

  this->SendU16(page, true);
  RUN_TASK_V(this->RecvACK());

  return TaskResult::Ok();
}

TaskResult STMBootLoader::DoExtendedErase(std::vector<uint16_t> pages) {
  this->SendCommand(this->erase);
  RUN_TASK_V(this->RecvACK());

  uint8_t* buffer = new uint8_t[pages.size() * 2 + 2];
  buffer[0] = (pages.size() - 1) >> 8;
  buffer[1] = (pages.size() - 1) & 0xff;
  for (int i = 0; i < pages.size(); i++) {
    buffer[2 + i * 2] = (pages[i] >> 8) & 0xff;
    buffer[2 + i * 2 + 1] = pages[i] & 0xff;
  }

  this->SendWithChecksum(buffer, pages.size() * 2 + 2);

  delete buffer;

  RUN_TASK_V(this->RecvACK());

  return TaskResult::Ok();
}

TaskResult STMBootLoader::DoErase(FlashPage page) {
  // TODO(syoch): Impl
  return ESP_ERR_NOT_SUPPORTED;
}
TaskResult STMBootLoader::DoErase(std::vector<uint16_t> pages) {
  // TODO(syoch): Impl
  return ESP_ERR_NOT_SUPPORTED;
}

STMBootLoader::STMBootLoader(gpio_num_t reset, gpio_num_t boot0,
                             uart_port_t num, int tx, int rx)
    : uart(num, tx, rx, 112500, UART_PARITY_EVEN), reset(reset), boot0(boot0) {
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

TaskResult STMBootLoader::Sync() {
  this->uart.Send((uint8_t*)"\x7f", 1);

  RUN_TASK(this->uart.RecvChar(), ret);
  if (ret == 0x79) {
    return TaskResult::Ok();
  } else if (ret == 0x1f) {
    this->in_error_state = true;
    return ESP_ERR_INVALID_STATE;
  } else {
    ESP_LOGW(TAG, "Unknown sync byte %#02x", ret);
    return TaskResult::Ok();
  }
}

TaskResult STMBootLoader::Get() {
  this->SendCommand(this->get);
  RUN_TASK_V(this->RecvACK());

  RUN_TASK_TO(this->uart.RecvChar(), this->ack);

  RUN_TASK(this->uart.RecvChar(), n);

  RUN_TASK(this->uart.RecvChar(), byte);
  this->version.major = byte >> 4;
  this->version.minor = byte & 0xf;

  char buf[16];
  RUN_TASK_V(this->uart.Recv((uint8_t*)buf, n, 100 / portTICK_PERIOD_MS));

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

  RUN_TASK_V(this->RecvACK());

  if (this->erase == 0x44) {
    this->use_extended_erase = true;
  }

  return TaskResult::Ok();
}

int STMBootLoader::WriteMemoryBlock(uint32_t address, uint8_t* buffer,
                                    size_t size) {
  ESP_LOGI(TAG, "Writing Memory at %08lx (%d bytes)", address, size);
  this->uart.Send((uint8_t*)"\x31\xce", 2);
  RUN_TASK_V(this->RecvACK());

  this->SendAddress(address);
  RUN_TASK_V(this->RecvACK());

  this->uart.SendChar(size - 1);

  uint8_t checksum = size - 1;
  for (int i = 0; i < size; i++) {
    checksum ^= buffer[i];
  }
  this->uart.Send((uint8_t*)buffer, size);
  this->uart.SendChar(checksum);

  RUN_TASK_V(this->RecvACK());
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

TaskResult STMBootLoader::Go(uint32_t address) {
  ESP_LOGI(TAG, "Go command");
  this->uart.Send((uint8_t*)"\x21\xde", 2);
  RUN_TASK_V(this->RecvACK());

  this->SendAddress(address);
  RUN_TASK_V(this->RecvACK());

  return TaskResult::Ok();
}

TaskResult STMBootLoader::Erase(FlashPage page) {
  if (this->use_extended_erase) {
    RUN_TASK_V(this->DoExtendedErase(page));
  } else {
    RUN_TASK_V(this->DoErase(page));
  }

  return TaskResult::Ok();
}

TaskResult STMBootLoader::Erase(std::vector<uint16_t> pages) {
  if (this->use_extended_erase) {
    RUN_TASK_V(this->DoExtendedErase(pages));
  } else {
    RUN_TASK_V(this->DoErase(pages));
  }

  return TaskResult::Ok();
}

TaskResult STMBootLoader::BulkErase(std::vector<uint16_t> pages) {
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
    RUN_TASK_V(this->Erase(sub_pages));
  }

  if (all_pages % 4 != 0) {
    std::vector<uint16_t> sub_pages;
    for (int j = 0; j < pages.size() - i; j++) {
      sub_pages.push_back(pages[i + j]);
    }
    ESP_LOGI(TAG, "Erasing pages... (All %d pages)", all_pages);
    RUN_TASK_V(this->Erase(sub_pages));
  }

  return TaskResult::Ok();
}

TaskResult STMBootLoader::Erase(uint32_t address, uint32_t length) {
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
    RUN_TASK_V(this->BulkErase(pages));
  } else {
    int bank1_erase_start = (address - 0x0800'0000) >> 11;
    int bank1_erase_end = (address + length - 0x08'000000) >> 11;

    std::vector<uint16_t> pages;
    for (int i = bank1_erase_start; i < bank1_erase_end; i++) {
      pages.push_back(i);
    }
    RUN_TASK_V(this->BulkErase(pages));
  }

  return TaskResult::Ok();
}