#include "server.hpp"

#include <connection/data_link/base.hpp>
#include <esp_log.h>
#include <lwip/sockets.h>
#include <optional>

#include "config/config.hpp"

class SocketWrapper : public connection::data_link::RecvAndSend {
 public:
  int socket;
  explicit SocketWrapper(int socket) : socket(socket) {}

  size_t Send(std::vector<uint8_t>& buf) override {
    auto ret = send(this->socket, buf.data(), buf.size(), 0);

    return ret;
  }
  using connection::data_link::RecvAndSend::Send;

  size_t Recv(std::vector<uint8_t>& buf,
              TickType_t timeout = 1000 / portTICK_PERIOD_MS) override {
    auto ret = recv(this->socket, buf.data(), buf.size(), 0);
    if (ret == 0) {
      throw connection::data_link::ConnectionClosedError();
    }

    return ret;
  }
};

namespace {
struct ClientHandler {
  enum class Opcode {
    BootBootLoader = 0x10,
    UploadProgram = 0x11,
    GoProgram = 0x12,

    GetUI = 0x20,
    ListenDataUpdate = 0x21,
    DataUpdate = 0x22,
  };

  Server& server;
  SocketWrapper client;

  void HandleClient() {
    std::string tag =
        "NW-W Handler[" + std::to_string(this->client.socket) + "]";
    const char* TAG = tag.c_str();

    using enum Opcode;
    while (true) {
      Opcode opcode;
      try {
        auto opcode_raw = this->client.RecvChar();
        opcode = static_cast<Opcode>(opcode_raw);
      } catch (connection::data_link::ConnectionClosedError&) {
        ESP_LOGI(TAG, "Connection closed.");
        break;
      }

      switch (opcode) {
        /*
         * The opcodes related STM32 BootLoader
         */
        case BootBootLoader: {
          config::Config::GetSTM32BootLoader()->BootBootLoader();
          config::Config::GetSTM32BootLoader()->Connect();
          vTaskDelay(200 / portTICK_PERIOD_MS);

          this->client.Send("OK");
          break;
        }

        case UploadProgram: {
          auto length = this->client.RecvU32();

          config::Config::GetSTM32BootLoader()->Erase(
              CONFIG_STM32_PROGRAM_START, length);

          std::vector<uint8_t> buffer(0x1000, 0);

          int end = CONFIG_STM32_PROGRAM_START + length;
          int ptr = CONFIG_STM32_PROGRAM_START;
          while (ptr < end) {
            auto received = this->client.Recv(buffer);
            buffer.resize(received);

            config::Config::GetSTM32BootLoader()->WriteMemory(ptr, buffer);
            ptr += received;
          }
          this->client.Send("OK");
          break;
        }

        case GoProgram: {
          config::Config::GetSTM32BootLoader()->Go(CONFIG_STM32_PROGRAM_START);
          this->client.Send("OK");
          break;
        }

          /*
           * The opcodes related to the debugger
           */

        case GetUI: {
          ESP_LOGI(TAG, "GetUI called");
          auto ui = config::Config::GetDebuggerMaster()->GetUI();

          this->client.SendU32(ui.size());
          this->client.Send(ui);
          break;
        }

        case ListenDataUpdate: {
          config::Config::GetDebuggerMaster()->AddListener(this->client.socket);
          this->client.RecvChar();
          config::Config::GetDebuggerMaster()->RemoveListener(
              this->client.socket);

          break;
        }

        case DataUpdate: {
          auto cid = this->client.RecvU32();
          auto len = (size_t)this->client.RecvU32();

          std::vector<uint8_t> tcp_buffer(len);
          this->client.RecvExactly(tcp_buffer);

          config::Config::GetDebuggerMaster()->DataUpdate(cid, tcp_buffer);
          break;
        }

        default: {
          ESP_LOGE(TAG, "Unknown opcode (opcode = 0x%02x)", (uint8_t)opcode);
          break;
        }
      }

      continue;
    }
  }
};

void ClientHandlerWrapper(ClientHandler* args) {
  static const char* TAG = "NW-W Handler Wrapper";
  try {
    args->HandleClient();
  } catch (connection::data_link::ConnectionClosedError&) {
    ESP_LOGI(TAG, "Connection closed.");
  } catch (std::exception& e) {
    ESP_LOGE(TAG, "Exception: %s", e.what());
  } catch (...) {
    ESP_LOGE(TAG, "Unknown exception.");
  }

  close(args->client.socket);
  delete args;

  vTaskDelete(nullptr);
}
}  // namespace

void Server::ClientLoop(void* obj) {
  auto server = static_cast<Server*>(obj);
  while (true) {
    auto client = accept(server->server_sock, nullptr, nullptr);
    if (client < 0) {
      ESP_LOGE(TAG, "Failed to accept client.");
      break;
    }

    ESP_LOGI(TAG, "Connection established.");
    auto args =
        new ClientHandler{.server = *server, .client = SocketWrapper(client)};
    xTaskCreate((TaskFunction_t)ClientHandlerWrapper, "Client", 4096, args, 1,
                nullptr);
  }
}

Server::Server() = default;

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
  ESP_LOGI(TAG, "Listening...");
  auto ret = ::listen(this->server_sock, 5);
  ESP_LOGD(TAG, "Listened: %d", ret);
  return !(ret < 0);
}

void Server::StartClientLoop() {
  xTaskCreate(&Server::ClientLoop, "Server Loop", 4096, this, 1, nullptr);
}

void Server::StartClientLoopAtForeground() { Server::ClientLoop(this); }
