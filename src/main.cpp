#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gptimer.h>
#include <driver/gpio.h>
#include <sdkconfig.h>
#include <esp_event.h>
#include <mdns.h>
#include "./measure.hpp"
#include "nvs_flash.h"
#include "libs/wifi.hpp"
#include "./config.hpp"
#include <lwip/sockets.h>

#include <optional>

#define WRITING_BAUD 115200

using namespace app;

app::Wifi network{};

class Server {
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
        printf("E: [NW-Wd] (%3d) Failed to receive data.\n", this->client);
        return -1;
      }
      if (len == 0) {
        printf("I: [NW-Wd] (%3d) Client disconnected.\n", this->client);
        return -1;
      }
      return len;
    }

    int TrySend(char* buf, int size) {
      auto sent = send(this->client, buf, size, 0);
      if (sent < 0) {
        printf("E: [NW-Wd] (%3d) Failed to send data.\n", client);
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
        printf("E: [NW-Wd] (%3d) Failed to receive register number.\n",
               this->client);
        return std::nullopt;
      }

      if (*reg < 0 || *reg >= 32) {
        printf("E: [NW-Wd] (%3d) Invalid register number (reg = %d)\n",
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
              printf("E: [NW-Wd] (%3d) Failed to receive the value\n", client);
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
              printf("E: [NW-Wd] (%3d) Failed to receive the value\n", client);
              break;
            }

            args->server.float_regs[*reg] = *val;

            break;
          }
          case Opcode::RegWriteString: {
            printf("I: [NW-Wd] (%3d) RegWriteString is not implemented\n",
                   client);
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
            printf("I: [NW-Wd] (%3d) RegReadString is not implemented\n",
                   client);
            auto reg = args->RecvRegisterNumber();
            if (!reg) break;
            break;
          }
          default: {
            printf("E: [NW-Wd] (%3d) Unknown opcode (opcode = %d)\n", client,
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
    printf("T: [NW-Wd] Server socket: %d\n", this->server_sock);
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
        printf("E: [NW-Wd] Failed to accept client.\n");
        continue;
      }

      printf("I: [NW-Wd] Connection established.\n");
      auto args = new ClientHandler{.server = *this, .client = client};
      xTaskCreate((TaskFunction_t)ClientHandler::HandleClient, "Client", 4096,
                  args, 1, NULL);
    }
  }
};

struct Decimal {
  int n;
  int d;
};

void BootStrap() {
  esp_err_t err;

  printf("I: [ Main] 1. Initializing Hardware/Softwares...\n");
  printf("I: [ Main]   1. GPIO\n");
  printf("I: [ Main]     1. BLINK_GPIO\n");
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(BLINK_GPIO, 1);
  printf("I: [ Main]   2. NVS Flush\n");
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  printf("I: [ Main]   3. Net Interface\n");
  ESP_ERROR_CHECK(esp_netif_init());

  printf("I: [ Main]   4. Event loop\n");
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  printf("I: [ Main]   5. Wi-Fi\n");
  network.Init();

  printf("I: [ Main]   6. MDns\n");
  ESP_ERROR_CHECK(mdns_init());

  printf("I: [ Main] 2. Setup Hardware/Softwares...\n");
  printf("I: [ Main]   1. Setting up WiFi\n");
  network.Setup();
  printf("I: [ Main]   2. Connecting Wi-Fi\n");
  network.Connect();
  printf("I: [ Main]   2. ...\n");
  network.WaitConnection();

  printf("I: [ Main]   3. Setting up MDns\n");
  mdns_hostname_set("esp32");
  mdns_instance_name_set("ESP32");
  mdns_service_add(NULL, "_tcp", "_tcp", 2000, NULL, 0);

  printf("I: [ Main] 3. Starting Network EventLoop Task.\n");
  network.StartEventLoop();
}

void Main() {
  Server server;
  printf("I: [ Main] 4. Starting Server\n");
  printf("I: [ Main]   1. bind...\n");
  if (!server.bind(4007)) {
    printf("I: [ Main] E: [NW-Wd] Failed to bind server.\n");
    return;
  }
  printf("I: [ Main]   2. listen...\n");
  if (!server.listen()) {
    printf("I: [ Main] E: [NW-Wd] Failed to listen server.\n");
    return;
  }
  printf("I: [ Main] 5. Enter the ClientLoop\n");
  server.ClientLoop();
}

extern "C" void app_main() {
  BootStrap();
  Main();
}