#include <stdio.h>
#include <driver/gpio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "simple_serial.hpp"

namespace simple_serial {
Tx::Tx(gpio_num_t data, gpio_num_t clock, gpio_num_t check)
    : data(data), clock(clock), check(check) {
  this->Init();
}
void Tx::Init() {
  gpio_set_direction(data, GPIO_MODE_OUTPUT);
  gpio_set_direction(clock, GPIO_MODE_OUTPUT);
  gpio_set_direction(check, GPIO_MODE_INPUT);
}

void Tx::Send(char *buffer, int len) {
  gpio_num_t data = this->data;
  gpio_num_t clock = this->clock;
  gpio_num_t check = this->check;
  Tx::SendBitWithWait(data, clock, check, 1);  // StartBit
  Tx::SendByte(data, clock, check, (len >> 24) & 0xff);
  Tx::SendByte(data, clock, check, (len >> 16) & 0xff);
  Tx::SendByte(data, clock, check, (len >> 8) & 0xff);
  Tx::SendByte(data, clock, check, len & 0xff);

  for (int i = 0; i < len; i++) {
    Tx::SendByte(data, clock, check, buffer[i]);
  }

  gpio_set_level(data, 0);
}
Rx::Rx(gpio_num_t data, gpio_num_t clock, gpio_num_t check)
    : data(data), clock(clock), check(check) {
  this->Init();
}

void Rx::Init() {
  gpio_set_direction(data, GPIO_MODE_INPUT);
  gpio_set_direction(clock, GPIO_MODE_INPUT);
  gpio_set_direction(check, GPIO_MODE_OUTPUT);
}

void Rx::Receive(char **buffer, int *len) {
  gpio_num_t data = this->data;
  gpio_num_t clock = this->clock;
  gpio_num_t check = this->check;

  while (!ReceiveBitWithWait(data, clock, check))
    ;

  int length = 0;
  length |= ReceiveByte(data, clock, check) << 24;
  length |= ReceiveByte(data, clock, check) << 16;
  length |= ReceiveByte(data, clock, check) << 8;
  length |= ReceiveByte(data, clock, check);
  if (length == 0) {
    *buffer = NULL;
    *len = 0;
    return;
  }

  *buffer = (char *)malloc(length);

  printf("RX << ");
  for (int i = 0; i < length; i++) {
    (*buffer)[i] = ReceiveByte(data, clock, check);
    printf("%c", (*buffer)[i]);
  }
  printf("\n");
  *len = length;
}
}  // namespace simple_serial
