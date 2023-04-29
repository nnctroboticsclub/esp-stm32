#include "sleep.hpp"

#include "libs/timer.hpp"
#include "esp_rom_sys.h"

void app::sleep_us_with_gptimer(uint64_t delay_us) {
  app::Timer timer{};

  while (1) {
    if (timer.GetRawCount() >= delay_us) {
      break;
    }
  }
}

void inline app::sleep_us_with_esp(uint64_t delay_us) {
  esp_rom_delay_us(delay_us);
}