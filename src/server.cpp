#include "server.hpp"

#include <esp_log.h>
#include <lwip/sockets.h>
#include <optional>

#include <result.hpp>
#include "config.hpp"

namespace {
struct ClientHandler {
  static constexpr const char* TAG = "NW-W C-handler";
  enum class Opcode {
    ReadMemoryU32,

    BootBootLoader = 0x10,
    UploadProgram,
    GoProgram,

    GetUI = 0x20,
    ListenDataUpdate = 0x21,
    DataUpdate = 0x22,
    UserMessage = 0x23,
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
  int TrySend(const char (&buf)[N]) {
    return this->TrySend((char*)buf, N);
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

  std::optional<int32_t> TryRecvInt() {
    auto a = this->TryRecvChar();
    auto b = this->TryRecvChar();
    auto c = this->TryRecvChar();
    auto d = this->TryRecvChar();

    // if a, b, c fails to receive, d will be failed
    if (!d) return std::nullopt;

    return (*a << 24) | (*b << 16) | (*c << 8) | *d;
  }

  void HandleClient() {
    int client = this->client;
    // Server& server = this->server;
    esp_err_t err = ESP_OK;
    while (1) {
      auto opcode_raw = this->TryRecvChar();
      if (!opcode_raw) break;

      Opcode opcode = static_cast<Opcode>(*opcode_raw);

      switch (opcode) {
        case Opcode::ReadMemoryU32: {
          auto addr = this->TryRecvInt();
          if (!addr) break;
          // buf[0] = 0x12;
          // buf[1] = (*addr >> 24) & 0xff;
          // buf[2] = (*addr >> 16) & 0xff;
          // buf[3] = (*addr >> 8) & 0xff;
          // buf[4] = (*addr) & 0xff;
          // TODO: Fix This
          // config::tx.Send(buf, 5);

          char* rx_buf = nullptr;
          int received = 0;
          // TODO: Fix This
          // config::rx.Receive(&rx_buf, &received);
          if (received != 4) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          this->TrySend(rx_buf, 4);
          free(rx_buf);

          break;
        }

        /*
         * The opcodes related STM32 BootLoader
         */
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
            this->TrySend("Boot Loader is in error state");
            break;
          }
          this->TrySend("OK");
          break;
        }

        case Opcode::UploadProgram: {
          auto length = this->TryRecvInt();
          if (!length) break;

          config::loader.Erase(USER_PROGRAM_START, *length);
          if (config::loader.in_error_state) {
            this->TrySend("Boot Loader is in error state");
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
          this->TrySend("OK");
          break;
        }

        case Opcode::GoProgram: {
          config::loader.Go(USER_PROGRAM_START);
          this->TrySend("OK");
          break;
        }

        /*
         * The opcodes related to the debugger
         */
        case Opcode::UserMessage: {
          auto length = this->TryRecvInt();
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
          // TODO: Fix This
          // config::tx.Send(tcp_buffer, *length + 1);

          delete[] tcp_buffer;

          char* rx_buffer = nullptr;
          int rx_received = 0;
          // TODO: Fix This
          // config::rx.Receive(&rx_buffer, &rx_received);
          this->TrySend(rx_buffer, rx_received);
          free(rx_buffer);
          break;
        }
        case Opcode::GetUI: {
          auto ret = config::debugger.GetUI();
          if (ret.IsErr()) {
            err = ret.Error();
            goto error;
          }

          auto ui = ret.Value();
          this->TrySend((char*)ui.data(), ui.size());
          break;
        }
        case Opcode::ListenDataUpdate: {
          config::debugger.AddListener(this->client);
          auto _ch = this->TryRecvChar();
          config::debugger.RemoveListener(this->client);

          break;
        }

        case Opcode::DataUpdate: {
        }

        default: {
          ESP_LOGE(TAG, "(%3d) Unknown opcode (opcode = %d)", client,
                   *opcode_raw);
          break;
        }
      }

      continue;
    error:
      ESP_LOGE(TAG, "(%3d) Failed %s", esp_err_to_name(err))
    }

    close(client);
    delete this;

    vTaskDelete(NULL);
  }
};

void ClientHandlerWrapper(ClientHandler* args) { args->HandleClient(); }
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
    xTaskCreate(ClientHandlerWrapper, "Client", 4096, args, 1, NULL);
  }
}

Server::Server() {}

void Server::MakeSocket() {
  this->server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  ESP_LOGD(TAG, "Server socket: %d", this->server_sock);
}

[[nodiscard]] bool Server::bind(int port) {
  sockaddr_in addr = {
      .sin_len = sizeof(addr),
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = htonl(INADDR_ANY),
      .sin_zero = {},
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
