#pragma once
#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <inttypes.h>

namespace app {
void sleep_us_with_gptimer(uint64_t delay_us);

void inline sleep_us_with_esp(uint64_t delay_us);

}  // namespace app

#endif