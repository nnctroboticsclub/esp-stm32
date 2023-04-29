#include "timer.hpp"
#include <driver/gptimer.h>

namespace app {
Timer::Timer() {
  gptimer_config_t config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = 1000000,  // microseconds.
      .flags = {.intr_shared = 1},
  };
  gptimer_new_timer(&config, &this->h);
  gptimer_enable(this->h);
}
Timer::~Timer() {
  gptimer_stop(this->h);
  gptimer_disable(this->h);
  gptimer_del_timer(this->h);
}
void Timer::SetRawCount(uint64_t count) {
  gptimer_set_raw_count(this->h, count);
}

}  // namespace app
