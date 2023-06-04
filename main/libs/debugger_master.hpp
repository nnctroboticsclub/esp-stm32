#include "uart.hpp"

#include <optional>
#include <vector>
#include <unordered_set>

#include "mutex.hpp"

class DebuggerMaster {
  static constexpr const char* TAG = "DebuggerMaster";
  Mutex<UART> uart;

  std::optional<std::vector<uint8_t>> ui_cache;
  std::unordered_set<int> listeners;

 public:
  DebuggerMaster(uart_port_t port, int tx, int rx);

  // Send a data update to the debugger
  TaskResult DataUpdate(int cid, uint8_t* value, size_t len);

  // Get the UI from the debugger
  Result<std::vector<uint8_t>> GetUI();

  // Wait for a command from the debugger
  TaskResult Idle();

  // Add a listener to the debugger
  void AddListener(int sock);

  // Remove a listener from the debugger
  void RemoveListener(int sock);
};