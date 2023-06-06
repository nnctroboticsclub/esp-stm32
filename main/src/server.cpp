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

  Result<int> TryRecv(uint8_t* buf, int size) {
    auto len = recv(this->client, buf, size, 0);
    if (len < 0) {
      ESP_LOGE(TAG, "(%3d) Failed to receive data.", this->client);
      return new ErrnoError(errno);
    }
    // if (len == 0) {
    //   ESP_LOGI(TAG, "(%3d) Client disconnected.", this->client);
    //   return ESP_ERR_INVALID_STATE;
    // }
    return Result<int>::Ok(len);
  }

  Result<int> TryRecv(char* buf, int size) {
    return this->TryRecv((uint8_t*)buf, size);
  }

  Result<int> TrySend(uint8_t* buf, int size) {
    auto sent = send(this->client, buf, size, 0);
    if (sent < 0) {
      ESP_LOGE(TAG, "(%3d) Failed to send data.", client);
      return new ErrnoError(errno);
    }
    return std::optional(sent);
  }

  template <int N>
  Result<int> TrySend(const uint8_t (&buf)[N]) {
    return this->TrySend((uint8_t*)buf, N);
  }
  template <int N>
  Result<int> TrySend(const char (&buf)[N]) {
    return this->TrySend((uint8_t*)buf, N);
  }

  template <int N>
  Result<int> TryRecv(uint8_t (&buf)[N]) {
    return this->TryRecv(buf, N);
  }

  Result<int> TryRecvChar() {
    char ch = 0;
    RUN_TASK(this->TryRecv((uint8_t*)&ch, 1), receipt);
    int ret = ch;
    if (ret < 0) {
      ret = 0x100 + ch;
    }

    if (receipt == 0) {
      ret = -1;
    }

    return Result<int>::Ok(ret);
  }

  TaskResult TrySendChar(char ch) {
    RUN_TASK_V(this->TrySend((uint8_t*)&ch, 1));
    return TaskResult::Ok();
  }

  TaskResult TrySendInt(int ch) {
    RUN_TASK_V(this->TrySendChar((ch >> 24) & 0xff));
    RUN_TASK_V(this->TrySendChar((ch >> 16) & 0xff));
    RUN_TASK_V(this->TrySendChar((ch >> 8) & 0xff));
    RUN_TASK_V(this->TrySendChar((ch >> 0) & 0xff));

    return TaskResult::Ok();
  }

  TaskResult TrySendFloat(float ch) { return this->TrySendInt(*(int*)&ch); }

  Result<int32_t> TryRecvInt() {
    RUN_TASK(this->TryRecvChar(), a);
    RUN_TASK(this->TryRecvChar(), b);
    RUN_TASK(this->TryRecvChar(), c);
    RUN_TASK(this->TryRecvChar(), d);

    return Result<int32_t>::Ok((a << 24) | (b << 16) | (c << 8) | d);
  }

  TaskResult HandleClient() {
    int client = this->client;
    while (1) {
      RUN_TASK(this->TryRecvChar(), opcode_raw);

      if (opcode_raw == -1) {
        ESP_LOGI(TAG, "(%3d) Client disconnected", client);
        break;
      }

      Opcode opcode = static_cast<Opcode>(opcode_raw);

      switch (opcode) {
        case Opcode::ReadMemoryU32: {
          ESP_LOGE(TAG, "Read Memory is not supported");
          return ESP_ERR_NOT_SUPPORTED;
          /*
          RUN_TASK(this->TryRecvInt(), addr);

          // buf[0] = 0x12;
          // buf[1] = (*addr >> 24) & 0xff;
          // buf[2] = (*addr >> 16) & 0xff;
          // buf[3] = (*addr >> 8) & 0xff;
          // buf[4] = (*addr) & 0xff;
          // TODO: Fix This
          // config::tx.Send(buf, 5);

          uint8_t* rx_buf = nullptr;
          int received = 0;
          // TODO: Fix This
          // config::rx.Receive(&rx_buf, &received);
          if (received != 4) {
            ESP_LOGE(TAG, "(%3d) Failed to receive the value", client);
            break;
          }

          RUN_TASK_V(this->TrySend(rx_buf, 4));
          free(rx_buf);

          */

          break;
        }

        /*
         * The opcodes related STM32 BootLoader
         */
#ifdef CONFIG_STM32_BOOTLOADER_DRIVER
        case Opcode::BootBootLoader: {
          config::loader.BootBootLoader();
          vTaskDelay(200 / portTICK_PERIOD_MS);
          RUN_TASK_V(config::loader.Sync());
          RUN_TASK_V(config::loader.Get());
          config::loader.GetVersion();

          ESP_LOGI(TAG, "Boot Loader version = %d.%d",
                   config::loader.GetVersion()->major,
                   config::loader.GetVersion()->minor);

          RUN_TASK_V(this->TrySend("OK"));
          break;
        }

        case Opcode::UploadProgram: {
          RUN_TASK(this->TryRecvInt(), length);

          RUN_TASK_V(config::loader.Erase(CONFIG_STM32_PROGRAM_START, length));

          uint8_t* tcp_buffer = new unsigned char[1024];
          if (tcp_buffer == nullptr) {
            ESP_LOGE(TAG, "(%3d) Failed to allocate memory", client);
            break;
          }

          int end = CONFIG_STM32_PROGRAM_START + length;
          int ptr = CONFIG_STM32_PROGRAM_START;
          while (ptr < end) {
            RUN_TASK(this->TryRecv(tcp_buffer, 1024), received);

            config::loader.WriteMemory(ptr, tcp_buffer, received);
            ptr += received;
          }
          RUN_TASK_V(this->TrySend("OK"));
          break;
        }

        case Opcode::GoProgram: {
          RUN_TASK_V(config::loader.Go(CONFIG_STM32_PROGRAM_START));
          RUN_TASK_V(this->TrySend("OK"));
          break;
        }
#else
        case Opcode::BootBootLoader: {
          ESP_LOGE(TAG, "BootBootLoader is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
        case Opcode::UploadProgram: {
          ESP_LOGE(TAG, "UploadProgram is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
        case Opcode::GoProgram: {
          ESP_LOGE(TAG, "GoProgram is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
#endif

        /*
         * The opcodes related to the debugger
         */
#ifdef CONFIG_STM32_REMOTE_CONTROLLER_DRIVER
        case Opcode::UserMessage: {
          RUN_TASK(this->TryRecvInt(), length);

          uint8_t* tcp_buffer = new uint8_t[length + 1];
          if (tcp_buffer == nullptr) {
            ESP_LOGE(TAG, "(%3d) Failed to allocate memory", client);
            break;
          }

          int tcp_received = 0;
          while (tcp_received < length) {
            RUN_TASK(this->TryRecv(tcp_buffer + 1 + tcp_received,
                                   length - tcp_received),
                     ret);
            tcp_received += ret;
          }

          if (tcp_received != length) break;

          tcp_buffer[0] = 0x80;
          // TODO: Fix This
          // config::tx.Send(tcp_buffer, length + 1);

          delete[] tcp_buffer;

          uint8_t* rx_buffer = nullptr;
          int rx_received = 0;
          // TODO: Fix This
          // config::rx.Receive(&rx_buffer, &rx_received);
          RUN_TASK_V(this->TrySend(rx_buffer, rx_received));
          free(rx_buffer);

          RUN_TASK_V(this->TrySend("OK"));
          break;
        }

        case Opcode::GetUI: {
          ESP_LOGI(TAG, "GetUI called");
          RUN_TASK(config::debugger.GetUI(), ui);

          RUN_TASK_V(this->TrySendInt(ui.size()));
          RUN_TASK_V(this->TrySend(ui.data(), ui.size()));
          break;
        }

        case Opcode::ListenDataUpdate: {
          config::debugger.AddListener(this->client);
          RUN_TASK_V(this->TryRecvChar());
          config::debugger.RemoveListener(this->client);

          break;
        }

        case Opcode::DataUpdate: {
          RUN_TASK(this->TryRecvInt(), cid);
          RUN_TASK(this->TryRecvInt(), length);

          uint8_t* tcp_buffer = new uint8_t[length];
          if (tcp_buffer == nullptr) {
            ESP_LOGE(TAG, "(%3d) Failed to allocate memory", client);
            break;
          }

          int tcp_received = 0;
          while (tcp_received < length) {
            RUN_TASK(
                this->TryRecv(tcp_buffer + tcp_received, length - tcp_received),
                ret);
            tcp_received += ret;
          }

          if (tcp_received != length) break;

          RUN_TASK_V(config::debugger.DataUpdate(cid, tcp_buffer, length));
          break;
        }
#else
        case Opcode::UserMessage: {
          ESP_LOGE(TAG, "UserMessage is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
        case Opcode::GetUI: {
          ESP_LOGE(TAG, "GetUI is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
        case Opcode::ListenDataUpdate: {
          ESP_LOGE(TAG, "ListenDataUpdate is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
        case Opcode::DataUpdate: {
          ESP_LOGE(TAG, "DataUpdate is Disabled in this build");
          return ESP_ERR_NOT_SUPPORTED;
        }
#endif

        default: {
          ESP_LOGE(TAG, "(%3d) Unknown opcode (opcode = %d)", client,
                   opcode_raw);
          break;
        }
      }

      continue;
    }

    return TaskResult::Ok();
  }
};

void ClientHandlerWrapper(ClientHandler* args) {
  auto ret = args->HandleClient();
  if (ret.IsErr()) {
    ESP_LOGE(ClientHandler::TAG, "(%3d) Error: %s", args->client,
             ret.Error().what());
    auto raw_error = ret.Error().GetRawError().get();
    ESP_LOGE(ClientHandler::TAG, "     Type: %s", typeid(raw_error).name());
  }
  close(args->client);
  delete args;

  vTaskDelete(NULL);
}
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
    xTaskCreate((TaskFunction_t)ClientHandlerWrapper, "Client", 4096, args, 1,
                NULL);
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
