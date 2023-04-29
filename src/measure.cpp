#include "measure.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "libs/timer.hpp"
#include "libs/gpio.hpp"
#include <driver/gpio.h>
#include "config.hpp"

namespace app::measure {
static uint64_t do_1cycle() {
  Timer timer{};
  app::DigitalOut pin{GPIO_NUM_2};

  uint64_t t = 2;
  for (size_t i = 0; i < 100; i++) {
    timer.SetRawCount(0);

    timer.Start();
    pin << 0;
    timer.Stop();
    pin << 1;

    uint64_t t2 = timer.GetRawCount();
    t = (t + t2) / 2;
  }

  return t;
}

static uint64_t do_calc() {
  printf("I: [Setup] 3. Measuring Pin Changing Time...\n");

  printf("I: [Setup]   1. ");
  uint64_t a = do_1cycle();
  printf("[Done: %lld]\n", a);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  printf("I: [Setup]   2. ");
  uint64_t b = do_1cycle();
  printf("[Done: %lld]\n", b);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  printf("I: [Setup]   3. ");
  uint64_t c = do_1cycle();
  printf("[Done: %lld]\n", c);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  printf("I: [Setup]   4. ");
  uint64_t d = do_1cycle();
  printf("[Done: %lld]\n", d);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  return (a + b + c + d) / 4;
}

uint64_t GetPinChangingTime() {
  static uint64_t cache = 0xffffffff;
  if (cache == 0xffffffff) {
    cache = do_calc();
  }
  return cache;
}
}  // namespace app::measure
