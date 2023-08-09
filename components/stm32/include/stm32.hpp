#pragma once

#include <connection/data_link/base.hpp>

#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <gpio_cxx.hpp>

#include "stm32/session.hpp"

namespace stm32 {

// Represents STM32 micro controller bootloader (On OL5)
class BootLoader {};

// Represents STM32 micro controller (On OL6)
class STM32 {};
}  // namespace stm32
