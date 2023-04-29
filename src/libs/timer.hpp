#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP

#include <driver/gptimer.h>

namespace app {
class Timer {
  gptimer_handle_t h;

 public:
  Timer();

  ~Timer();

  inline void Start() { gptimer_start(this->h); }

  inline void Stop() { gptimer_stop(this->h); }

  inline uint64_t GetRawCount() {
    uint64_t t2;
    gptimer_get_raw_count(this->h, &t2);
    return t2;
  }

  void SetRawCount(uint64_t count);
};
}  // namespace app

#endif