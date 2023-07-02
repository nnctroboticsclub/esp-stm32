#include "debugger_master.hpp"

#include <esp_log.h>

#include <lwip/sockets.h>

using connection::data_link::UART;

DebuggerMaster::DebuggerMaster(uart_port_t port, int tx, int rx)
    : uart_mutex(UART(port)), ui_cache{} {
  auto uart = this->uart_mutex.Lock();
  uart->InstallDriver(tx, rx, 9600, UART_PARITY_DISABLE);
}

void DebuggerMaster::DataUpdate(uint32_t cid, std::vector<uint8_t> &buf) {
  auto uart = this->uart_mutex.Lock();

  uart->SendChar(0x02);
  uart->SendU32(cid);
  uart->SendU32(buf.size());

  uart->Send(buf);
}

std::vector<uint8_t> &DebuggerMaster::GetUI() {
  auto uart = this->uart_mutex.Lock();

  ESP_LOGI(TAG, "Getting UI");
  if (!this->ui_cache.empty()) {
    ESP_LOGI(TAG, "Using cached UI");
    return this->ui_cache;
  }

  uart->SendChar(0x01);

  auto c = uart->RecvChar(1000 / portTICK_PERIOD_MS);
  if (c != 0) {
    ESP_LOGE(TAG, "Invalid UI command: %#02x", c);
    throw InvalidCommand();
  }

  uint32_t length = uart->RecvU32();
  ESP_LOGI(TAG, "UI length: %ld", length);

  this->ui_cache.resize((uint)length);
  uart->RecvExactly(ui_cache);

  ESP_LOGI(TAG, "UI received");
  return this->ui_cache;
}

static void CopyU32ToVec(std::vector<uint8_t> dst, size_t offset,
                         uint32_t value) {
  dst[offset + 0] = (value >> 24) & 0xff;
  dst[offset + 1] = (value >> 16) & 0xff;
  dst[offset + 2] = (value >> 8) & 0xff;
  dst[offset + 3] = value & 0xff;
}

void DebuggerMaster::Idle() {
  auto uart = this->uart_mutex.Lock();

  auto buffer_length = uart->GetRXBufferDataLength();
  if (buffer_length <= 0) {
    return;
  }

  auto op = uart->RecvChar();

  switch (op) {
    case 2: {
      auto cid = uart->RecvU32();
      auto len = uart->RecvU32();

      std::vector<uint8_t> buf(uint(len + 8));

      std::vector data(buf.begin() + 8, buf.end());
      uart->RecvExactly(data, 200 / portTICK_PERIOD_MS);

      CopyU32ToVec(buf, 0, cid);
      CopyU32ToVec(buf, 4, len);

      for (auto listener : listeners) {
        send(listener, buf.data(), buf.size(), 0);
      }

      break;
    }
    default: {
      ESP_LOGW(TAG, "Unknown Opcode %#02x", op);
    }
  }
}

void DebuggerMaster::AddListener(int sock) { this->listeners.insert(sock); }
void DebuggerMaster::RemoveListener(int sock) { this->listeners.erase(sock); }