#pragma once

#include <esp_log.h>
#include <optional>

class Server {
  static constexpr const char* TAG = "NW-Wd";
  int server_sock;

 public:
  int int_regs[32];
  float float_regs[32];

 private:
  static void ClientLoop(void* obj);

 public:
  Server();

  void MakeSocket();

  [[nodiscard]] bool bind(int port);
  [[nodiscard]] bool listen();

  void StartClientLoop();
  void StartClientLoopAtForeground();
};
