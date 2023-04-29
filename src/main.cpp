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

#define WRITING_BAUD 115200

using namespace app;

app::Wifi network{};

class Server {
  int server_sock;

 private:
  struct ClientHandlerArguments {
    Server& server;
    int client;
  };

  static void HandleClient(ClientHandlerArguments* args) {
    int client = args->client;
    // Server& server = args->server;

    // Simple Echo back
    char buf[1024];

    while (1) {
      auto len = recv(client, buf, sizeof(buf), 0);
      if (len < 0) {
        printf("E: [NW-Wd] (%3d) Failed to receive data.\n", client);
        break;
      }
      if (len == 0) {
        printf("I: [NW-Wd] (%3d) Client disconnected.\n", client);
        break;
      }

      printf("I: [NW-Wd] (%3d) Received %d bytes.\n", client, len);
      auto sent = send(client, buf, len, 0);
      if (sent < 0) {
        printf("E: [NW-Wd] (%3d) Failed to send data.\n", client);
        break;
      }
      printf("I: [NW-Wd] (%3d) Sent %d bytes.\n", client, sent);
    }

    close(client);
    delete args;

    vTaskDelete(NULL);
  }

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
      printf("I: [NW-Wd] Accepted client.\n");

      auto args = new ClientHandlerArguments{.server = *this, .client = client};
      xTaskCreate((TaskFunction_t)HandleClient, "Client", 4096, args, 1, NULL);
      printf("T: [NW-Wd] Created client task.\n");
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