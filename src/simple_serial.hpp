#include <stdio.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace simple_serial {
/**
 * Protocol: [1; 1] [data byte; 8]
 *
 *
 */

class Tx {
  gpio_num_t data;
  gpio_num_t clock;
  gpio_num_t check;

  static inline void SendBit(gpio_num_t data, gpio_num_t clock,
                             gpio_num_t check, bool value) {
    gpio_set_level(data, value);

    gpio_set_level(clock, 1);
    while (!gpio_get_level(check))
      ;
    gpio_set_level(clock, 0);
    while (gpio_get_level(check))
      ;
  }
  static inline void SendBitWithWait(gpio_num_t data, gpio_num_t clock,
                                     gpio_num_t check, bool value) {
    gpio_set_level(data, value);

    gpio_set_level(clock, 1);
    while (!gpio_get_level(check)) vTaskDelay(1);
    gpio_set_level(clock, 0);
    while (gpio_get_level(check)) vTaskDelay(1);
  }

  static inline void SendByte(gpio_num_t data, gpio_num_t clock,
                              gpio_num_t check, uint8_t byte) {
    for (int i = 0; i < 8; i++) {
      Tx::SendBit(data, clock, check, (byte >> (7 - i)) & 1);
    }
  }

 public:
  Tx(gpio_num_t data, gpio_num_t clock, gpio_num_t check);

  void Send(char *buffer, int len);
};

class Rx {
  gpio_num_t data;
  gpio_num_t clock;
  gpio_num_t check;

  static inline bool ReceiveBit(gpio_num_t data, gpio_num_t clock,
                                gpio_num_t check) {
    gpio_set_level(check, 0);
    while (!gpio_get_level(clock))
      ;

    bool value = gpio_get_level(data);

    gpio_set_level(check, 1);
    while (gpio_get_level(clock))
      ;

    return value;
  }

  static inline bool ReceiveBitWithWait(gpio_num_t data, gpio_num_t clock,
                                        gpio_num_t check) {
    gpio_set_level(check, 0);
    while (!gpio_get_level(clock)) vTaskDelay(1);

    bool value = gpio_get_level(data);

    gpio_set_level(check, 1);
    while (gpio_get_level(clock)) vTaskDelay(1);

    return value;
  }

  static inline uint8_t ReceiveByte(gpio_num_t data, gpio_num_t clock,
                                    gpio_num_t check) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
      byte = (byte << 1) | ReceiveBit(data, clock, check);
    }
    return byte;
  }

 public:
  Rx(gpio_num_t data, gpio_num_t clock, gpio_num_t check);

  void Receive(char **buffer, int *len);
};
}  // namespace simple_serial
