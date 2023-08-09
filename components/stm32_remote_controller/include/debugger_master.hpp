#pragma once
#include <connection/data_link/uart.hpp>

#include <optional>
#include <vector>
#include <unordered_set>
#include <stdexcept>

#include <mutex.hpp>

class InvalidCommand : public std::runtime_error {
 public:
  InvalidCommand() : std::runtime_error("Invalid command") {}
};
class DebuggerMaster {
  static constexpr const char* TAG = "DebuggerMaster";
  async::Mutex<connection::data_link::UART> uart_mutex;

  std::vector<uint8_t> ui_cache;
  std::unordered_set<int> listeners;

 public:
  DebuggerMaster(uart_port_t port, int tx, int rx);

  // Send a data update to the debugger
  void DataUpdate(uint32_t cid, std::vector<uint8_t>& buf);

  // Get the UI from the debugger
  std::vector<uint8_t>& GetUI();

  // Wait for a command from the debugger
  void Idle();

  // Add a listener to the debugger
  void AddListener(int sock);

  // Remove a listener from the debugger
  void RemoveListener(int sock);
};