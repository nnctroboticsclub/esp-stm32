
#include "uart.hpp"
#include <inttypes.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>

#include <vector>

class STMBootLoader {
  static constexpr const char* TAG = "STMBootLoader";

 public:
  struct Version {
    bool is_valid = false;
    uint8_t major;
    uint8_t minor;
    uint8_t option1;
    uint8_t option2;
  };

  enum class ACK { ACK, NACK };

  bool in_error_state = false;

 private:
  bool use_extended_erase = false;

  uint8_t get = 0x00;
  uint8_t get_version = 0x01;
  uint8_t get_id = 0x02;
  uint8_t read_memory = 0x11;
  uint8_t go = 0x21;
  uint8_t write_memory = 0x31;
  uint8_t erase = 0x43;
  uint8_t write_protect = 0x63;
  uint8_t write_unprotect = 0x73;
  uint8_t readout_protect = 0x82;
  uint8_t readout_unprotect = 0x92;
  uint8_t ack = 0x79;

  using FlashPage = uint16_t;
  static constexpr const FlashPage global = 0xffff;
  static constexpr const FlashPage bank1 = 0xfffe;
  static constexpr const FlashPage bank2 = 0xfffd;

  Version version;

  UART uart;
  gpio_num_t reset, boot0;

  ACK RecvACK(TickType_t timeout = 100 / portTICK_PERIOD_MS);

  void SendWithChecksum(char* buf, size_t size);

  void SendU16(uint8_t high, uint8_t low, bool with_checksum = false);

  void SendU16(uint16_t value, bool with_checksum = false);

  void SendCommand(uint8_t command);

  void SendAddress(uint32_t address);

  void DoGetVersion();

  void DoExtendedErase(FlashPage page);

  void DoExtendedErase(std::vector<uint16_t> pages);

  void DoErase(FlashPage page);
  void DoErase(std::vector<uint16_t> pages);

 public:
  STMBootLoader(gpio_num_t reset, gpio_num_t boot0, uart_port_t num, int tx,
                int rx);

  Version* GetVersion();

  void BootBootLoader();

  void Sync();

  void Get();

  int WriteMemoryBlock(uint32_t address, uint8_t* buffer, size_t size);

  int WriteMemory(uint32_t address, unsigned char* buffer, size_t size);

  void Go(uint32_t address);

  void Erase(FlashPage page);

  void Erase(std::vector<uint16_t> pages);

  void BulkErase(std::vector<uint16_t> pages);

  void Erase(uint32_t address, uint32_t length);
};