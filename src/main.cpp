#include <stdio.h>

#include "simple_serial.hpp"

#include <driver/gpio.h>
#include "./config.hpp"

#define WRITING_BAUD 115200

extern "C" void app_main() {
  // simple_serial::Rx rx(rx_data, rx_clock, rx_check);
  simple_serial::Tx tx(tx_data, tx_clock, tx_check);

  char buffer[1024]{};
  while (1) {
    printf("Send 1kB data.\n");
    tx.Send(buffer, 1024);
  }

  // GPIOTest();
}