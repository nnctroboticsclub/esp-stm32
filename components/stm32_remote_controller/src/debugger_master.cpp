#include "debugger_master.hpp"

#include <esp_log.h>

#include <lwip/sockets.h>

DebuggerMaster::DebuggerMaster(uart_port_t port, int tx, int rx)
    : uart(UART(port, tx, rx, 9600, UART_PARITY_DISABLE)),
      ui_cache(std::nullopt) {}

TaskResult DebuggerMaster::DataUpdate(int cid, uint8_t* value, size_t len) {
  RUN_TASK(this->uart.Lock(), uart);

  uint8_t buf[10];
  buf[0] = 0x02;
  buf[1] = (cid >> 24) & 0xff;
  buf[2] = (cid >> 16) & 0xff;
  buf[3] = (cid >> 8) & 0xff;
  buf[4] = (cid >> 0) & 0xff;

  buf[5] = (len >> 24) & 0xff;
  buf[6] = (len >> 16) & 0xff;
  buf[7] = (len >> 8) & 0xff;
  buf[8] = (len >> 0) & 0xff;

  uart->Send(buf, 9);

  uart->Send(value, len);

  return TaskResult::Ok();
}

Result<std::vector<uint8_t>> DebuggerMaster::GetUI() {
  RUN_TASK(this->uart.Lock(), uart);

  ESP_LOGI(TAG, "Getting UI");
  if (ui_cache.has_value()) {
    ESP_LOGI(TAG, "Using cached UI");
    return Result<std::vector<uint8_t>>::Ok(ui_cache.value());
  }

  uart->Send((uint8_t*)"\x01", 1);

  RUN_TASK(uart->RecvChar(1000 / portTICK_PERIOD_MS), c);
  if (c != 0) {
    ESP_LOGE(TAG, "Invalid UI command: %#02x", c);
    return ESP_ERR_INVALID_RESPONSE;
  }

  uint8_t length_buf[4]{};

  RUN_TASK_V(uart->RecvExactly(length_buf, 4, 200 / portTICK_PERIOD_MS));

  uint32_t length = length_buf[0] << 24 | length_buf[1] << 16 |
                    length_buf[2] << 8 | length_buf[3];
  ESP_LOGI(TAG, "UI length: %ld", length);

  ui_cache = std::vector<uint8_t>(length);

  RUN_TASK_V(uart->RecvExactly(ui_cache.value().data(), length));

  ESP_LOGI(TAG, "UI received");
  return Result<std::vector<uint8_t>>::Ok(ui_cache.value());
}

TaskResult DebuggerMaster::Idle() {
  RUN_TASK(this->uart.Lock(), uart);

  auto buffer_length = uart->GetRXBufferDataLength();
  if (buffer_length <= 0) {
    return TaskResult::Ok();
  }

  RUN_TASK(uart->RecvChar(1000 / portTICK_PERIOD_MS), op);

  switch (op) {
    case 2: {
      uint8_t cid_buf[4];
      uint8_t len_buf[4];
      uint32_t len;

      RUN_TASK_V(uart->RecvExactly(cid_buf, 4, 200 / portTICK_PERIOD_MS));
      RUN_TASK_V(uart->RecvExactly(len_buf, 4, 200 / portTICK_PERIOD_MS));

      len = len_buf[0] << 24 | len_buf[1] << 16 | len_buf[2] << 8 | len_buf[3];

      uint8_t* buffer = (uint8_t*)malloc(len + 8);
      RUN_TASK_V(uart->RecvExactly(buffer + 8, len, 200 / portTICK_PERIOD_MS));

      memcpy(buffer, cid_buf, 4);
      memcpy(buffer + 4, len_buf, 4);

      for (auto listener : listeners) {
        auto ret = send(listener, buffer, len + 8, 0);
      }

      free(buffer);
      break;
    }
    default: {
      ESP_LOGW(TAG, "Unknown Opcode %#02x", op);
    }
  }
  return TaskResult::Ok();
}

void DebuggerMaster::AddListener(int sock) { this->listeners.insert(sock); }
void DebuggerMaster::RemoveListener(int sock) { this->listeners.erase(sock); }