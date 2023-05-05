#include <esp_log.h>
#include <lwip/sockets.h>
#include <optional>

class Server {
  static constexpr const char* TAG = "NW-Wd";
  int server_sock;
  int int_regs[32];
  float float_regs[32];

 private:
  struct ClientHandler {
    enum class Opcode {
      RegWriteInt,
      RegWriteFloat,
      RegWriteString,
      RegReadInt,
      RegReadFloat,
      RegReadString,
    };

    Server& server;
    int client;

    int TryRecv(char* buf, int size) {
      auto len = recv(this->client, buf, size, 0);
      if (len < 0) {
        ESP_LOGE(TAG, "(%3d) Failed to receive data.\n", this->client);
        return -1;
      }
      if (len == 0) {
        ESP_LOGI(TAG, "(%3d) Client disconnected.\n", this->client);
        return -1;
      }
      return len;
    }

    int TrySend(char* buf, int size) {
      auto sent = send(this->client, buf, size, 0);
      if (sent < 0) {
        ESP_LOGE(TAG, "(%3d) Failed to send data.\n", client);
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
      char ch;
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
        ESP_LOGE(TAG, "(%3d) Failed to receive register number.\n",
                 this->client);
        return std::nullopt;
      }

      if (*reg >= 32) {
        ESP_LOGE(TAG, "(%3d) Invalid register number (reg = %d)\n",
                 this->client, *reg);
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
              ESP_LOGE(TAG, "(%3d) Failed to receive the value\n", client);
              break;
            }

            args->server.int_regs[*reg] = *val;

            break;
          }
          case Opcode::RegWriteFloat: {
            auto reg = args->RecvRegisterNumber();
            if (!reg) break;

            auto val = args->TryRecvFloat();
            if (!val) {
              ESP_LOGE(TAG, "(%3d) Failed to receive the value\n", client);
              break;
            }

            args->server.float_regs[*reg] = *val;

            break;
          }
          case Opcode::RegWriteString: {
            ESP_LOGI(TAG, "(%3d) RegWriteString is not implemented\n", client);
            break;
          }
          case Opcode::RegReadInt: {
            auto reg = args->RecvRegisterNumber();
            if (!reg) break;

            auto val = args->server.int_regs[*reg];
            args->TrySendInt(val);

            break;
          }
          case Opcode::RegReadFloat: {
            auto reg = args->RecvRegisterNumber();
            if (!reg) break;

            auto val = args->server.float_regs[*reg];
            args->TrySendFloat(val);

            break;
          }
          case Opcode::RegReadString: {
            ESP_LOGI(TAG, "(%3d) RegReadString is not implemented\n", client);
            auto reg = args->RecvRegisterNumber();
            if (!reg) break;
            break;
          }
          default: {
            ESP_LOGE(TAG, "(%3d) Unknown opcode (opcode = %d)\n", client,
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

 public:
  Server() : server_sock(socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) {
    ESP_LOGD(TAG, "Server socket: %d\n", this->server_sock);
  }

  [[nodiscard]] bool bind(int port) {
    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = htonl(INADDR_ANY),
    };
    auto ret = ::bind(this->server_sock, (sockaddr*)&addr, sizeof(addr));
    return !(ret < 0);
  }

  [[nodiscard]] bool listen() {
    auto ret = ::listen(this->server_sock, 5);
    return !(ret < 0);
  }

  void ClientLoop() {
    while (1) {
      auto client = accept(this->server_sock, NULL, NULL);
      if (client < 0) {
        ESP_LOGE(TAG, "Failed to accept client.\n");
        continue;
      }

      ESP_LOGI(TAG, "Connection established.\n");
      auto args = new ClientHandler{.server = *this, .client = client};
      xTaskCreate((TaskFunction_t)ClientHandler::HandleClient, "Client", 4096,
                  args, 1, NULL);
    }
  }
};
