#include "server.hpp"

#include <esp_log.h>
#include <lwip/sockets.h>
#include <optional>

#include "config.hpp"

namespace {
struct ClientHandler {
  static constexpr const char* TAG = "NW-W C-handler";
  enum class Opcode {
    RegWriteInt,
    RegWriteFloat,
    RegWriteString,
    RegReadInt,
    RegReadFloat,
    RegReadString,
    ReadMemoryU32,
    BootBootLoader,
    UploadProgram,
    GoProgram,
    UserMessage = 0x80,
  };

  Server& server;
  int client;

  int TryRecv(char* buf, int size) {
    auto len = recv(this->client, buf, size, 0);
    if (len < 0) {
      ESP_LOGE(TAG, "(%3d) Failed to receive data.", this->client);
      return -1;
    }
    if (len == 0) {
      ESP_LOGI(TAG, "(%3d) Client disconnected.", this->client);
      return -1;
    }
    return len;
  }

  int TrySend(char* buf, int size) {
    auto sent = send(this->client, buf, size, 0);
    if (sent < 0) {
      ESP_LOGE(TAG, "(%3d) Failed to send data.", client);
      return -1;
    }
    return sent;
  }

  template <int N>
  int TrySend(char (&buf)[N]) {
    return this->TrySend(buf, N);
  }

  template <int N>
  int TryRecv(char (&buf)[N]) {
    return this->TryRecv(buf, N);
  }

  std::optional<char> TryRecvChar() {
    char ch = 0;
    auto len = this->TryRecv(&ch, 1);
    if (len == -1) return std::nullopt;
    return ch;
  }

  int TrySendChar(char ch) { return this->TrySend(&ch, 1); }

  int TrySendInt(int ch) {
    this->TrySendChar((ch >> 24) & 0xff);
    this->TrySendChar((ch >> 16) & 0xff);
    this->TrySendChar((ch >> 8) & 0xff);
    return this->TrySendChar(ch & 0xff) ? 4 : -1;
  }

  int TrySendFloat(float ch) { return this->TrySendInt(*(int*)&ch); }

  std::optional<int> RecvRegisterNumber() {
    auto reg = this->TryRecvChar();
    if (!reg) {
      ESP_LOGE(TAG, "(%3d) Failed to receive register number.", this->client);
      return std::nullopt;
    }

    if (*reg >= 32) {
      ESP_LOGE(TAG, "(%3d) Invalid register number (reg = %d)", this->client,
               *reg);
      return std::nullopt;
    }

    return reg;
  }

  std::optional<int32_t> TryRecvInt() {
    auto a = this->TryRecvChar();
    auto b = this->TryRecvChar();
    auto c = this->TryRecvChar();
    auto d = this->TryRecvChar();

    // if a, b, c fails to receive, d will be failed
    if (!d) return std::nullopt;

    return (*a << 24) | (*b << 16) | (*c << 8) | *d;
  }

  std::optional<float> TryRecvFloat() {
    auto ieee = this->TryRecvInt();
    if (!ieee) return std::nullopt;

    return *(float*)&ieee;
  }

  static void HandleClient(ClientHandler* args) {
    int client = args->client;
    // Server& server = args->server;
    char buf[1024];
    while (1) {
      auto opcode_raw = args->TryRecvChar();
      if (!opcode_raw) break;

      Opcode opcode = static_cast<Opcode>(*opcode_raw);

      switch (opcode) {
        case Opcode::RegWriteInt: {
          auto reg = args->RecvRegisterNumber();
          if (!reg) break;

          auto val = args->TryRecvInt();
          if (!val) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          args->server.int_regs[*reg] = *val;
          buf[0] = 0x00;
          buf[1] = *reg;
          buf[2] = (*val >> 24) & 0xff;
          buf[3] = (*val >> 16) & 0xff;
          buf[4] = (*val >> 8) & 0xff;
          buf[5] = *val & 0xff;
          config::tx.Send(buf, 6);

          break;
        }
        case Opcode::RegWriteFloat: {
          auto reg = args->RecvRegisterNumber();
          if (!reg) break;

          auto val = args->TryRecvFloat();
          if (!val) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          args->server.float_regs[*reg] = *val;

          float raw = *val;
          uint32_t ieee = *(uint32_t*)&raw;

          buf[0] = 0x01;
          buf[1] = *reg;
          buf[2] = (ieee >> 24) & 0xff;
          buf[3] = (ieee >> 16) & 0xff;
          buf[4] = (ieee >> 8) & 0xff;
          buf[5] = ieee & 0xff;
          config::tx.Send(buf, 6);

          break;
        }
        case Opcode::RegWriteString: {
          ESP_LOGI(TAG, "(%3d) RegWriteString is not implemented", client);
          break;
        }
        case Opcode::RegReadInt: {
          auto reg = args->RecvRegisterNumber();
          if (!reg) break;

          buf[0] = 0x10;
          buf[1] = *reg;
          config::tx.Send(buf, 2);

          char* rx_buf;
          int received;
          config::rx.Receive(&rx_buf, &received);
          if (received != 4) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          int val_nucleo = (rx_buf[0] << 24) | (rx_buf[1] << 16) |
                           (rx_buf[2] << 8) | rx_buf[3];
          free(rx_buf);

          auto val_local = args->server.int_regs[*reg];
          if (val_nucleo != val_local) {
            ESP_LOGI(TAG, "(%3d) Value mismatch (local = %d, nucleo = %d)",
                     client, val_local, val_nucleo);
            args->server.int_regs[*reg] = val_nucleo;
          }
          args->TrySendInt(val_nucleo);

          break;
        }
        case Opcode::RegReadFloat: {
          auto reg = args->RecvRegisterNumber();
          if (!reg) break;

          buf[0] = 0x10;
          buf[1] = *reg;
          config::tx.Send(buf, 2);

          char* rx_buf;
          int received;
          config::rx.Receive(&rx_buf, &received);
          if (received != 4) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          uint32_t val_nucleo_ieee = (rx_buf[0] << 24) | (rx_buf[1] << 16) |
                                     (rx_buf[2] << 8) | rx_buf[3];
          free(rx_buf);
          float val_nucleo = *(float*)&val_nucleo_ieee;
          auto val_local = args->server.float_regs[*reg];

          if (val_nucleo != val_local) {
            ESP_LOGI(TAG, "(%3d) Value mismatch (local = %f, nucleo = %f)",
                     client, val_local, val_nucleo);
            args->server.float_regs[*reg] = val_nucleo;
          }
          args->TrySendFloat(val_nucleo);

          break;
        }
        case Opcode::ReadMemoryU32: {
          auto addr = args->TryRecvInt();
          if (!addr) break;
          buf[0] = 0x12;
          buf[1] = (*addr >> 24) & 0xff;
          buf[2] = (*addr >> 16) & 0xff;
          buf[3] = (*addr >> 8) & 0xff;
          buf[4] = (*addr) & 0xff;
          config::tx.Send(buf, 5);

          char* rx_buf;
          int received;
          config::rx.Receive(&rx_buf, &received);
          if (received != 4) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          args->TrySend(rx_buf, 4);
          free(rx_buf);

          break;
        }
        case Opcode::BootBootLoader: {
          config::loader.BootBootLoader();
          vTaskDelay(200 / portTICK_PERIOD_MS);
          config::loader.Sync();
          config::loader.Get();
          config::loader.GetVersion();

          ESP_LOGI(TAG, "Boot Loader version = %d.%d",
                   config::loader.GetVersion()->major,
                   config::loader.GetVersion()->minor);

          if (config::loader.in_error_state) {
            args->TrySend("Boot Loader is in error state", 30);
            break;
          }
          args->TrySend("OK", 2);
          break;
        }

        case Opcode::UploadProgram: {
          auto length = args->TryRecvInt();
          if (!length) break;

          config::loader.Erase(USER_PROGRAM_START, *length);
          if (config::loader.in_error_state) {
            args->TrySend("Boot Loader is in error state", 30);
            break;
          }

          unsigned char* tcp_buffer = new unsigned char[1024];
          if (tcp_buffer == nullptr) {
            ESP_LOGE(TAG, "(%3d) Failed to allocate memory", client);
            break;
          }

          int end = USER_PROGRAM_START + *length;
          int ptr = USER_PROGRAM_START;
          while (ptr < end) {
            auto received = recv(client, tcp_buffer, 1024, 0);
            if (received < 0) {
              ESP_LOGE(TAG, "(%3d) Failed to receive the message", client);
              break;
            }

            config::loader.WriteMemory(ptr, tcp_buffer, received);
            ptr += received;
          }
          args->TrySend("OK", 2);
          break;
        }
        case Opcode::GoProgram: {
          config::loader.Go(USER_PROGRAM_START);
          args->TrySend("OK", 2);
          break;
        }
        case Opcode::RegReadString: {
          ESP_LOGI(TAG, "(%3d) RegReadString is not implemented", client);
          auto reg = args->RecvRegisterNumber();
          if (!reg) break;
          break;
        }
        case Opcode::UserMessage: {
          auto length = args->TryRecvInt();
          if (!length) break;

          char* tcp_buffer = new char[*length + 1];
          if (tcp_buffer == nullptr) {
            ESP_LOGE(TAG, "(%3d) Failed to allocate memory", client);
            break;
          }

          int tcp_received = 0;
          while (tcp_received < *length) {
            auto ret = recv(client, tcp_buffer + 1 + tcp_received,
                            *length - tcp_received, 0);
            if (ret < 0) {
              ESP_LOGE(TAG, "(%3d) Failed to receive the message", client);
              break;
            }
            tcp_received += ret;
          }

          if (tcp_received != *length) break;

          tcp_buffer[0] = 0x80;
          config::tx.Send(tcp_buffer, *length + 1);

          delete[] tcp_buffer;

          char* rx_buffer;
          int rx_received;
          config::rx.Receive(&rx_buffer, &rx_received);
          args->TrySend(rx_buffer, rx_received);
          free(rx_buffer);
          break;
        }
        default: {
          ESP_LOGE(TAG, "(%3d) Unknown opcode (opcode = %d)", client,
                   *opcode_raw);
          break;
        }
      }
    }

    close(client);
    delete args;

    vTaskDelete(NULL);
  }
};
}  // namespace

void Server::ClientLoop(void* obj) {
  auto server = reinterpret_cast<Server*>(obj);
  while (1) {
    auto client = accept(server->server_sock, NULL, NULL);
    if (client < 0) {
      ESP_LOGE(TAG, "Failed to accept client.");
      continue;
    }

    ESP_LOGI(TAG, "Connection established.");
    auto args = new ClientHandler{.server = *server, .client = client};
    xTaskCreate((TaskFunction_t)ClientHandler::HandleClient, "Client", 4096,
                args, 1, NULL);
  }
}

Server::Server() {}

void Server::MakeSocket() {
  this->server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  ESP_LOGD(TAG, "Server socket: %d", this->server_sock);
}

[[nodiscard]] bool Server::bind(int port) {
  sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = htonl(INADDR_ANY),
  };
  auto ret = ::bind(this->server_sock, (sockaddr*)&addr, sizeof(addr));
  return !(ret < 0);
}

[[nodiscard]] bool Server::listen() {
  auto ret = ::listen(this->server_sock, 5);
  return !(ret < 0);
}

void Server::StartClientLoop() {
  xTaskCreate(&Server::ClientLoop, "Server Loop", 4096, this, 1, NULL);
}

void Server::StartClientLoopAtForeground() { Server::ClientLoop(this); }
