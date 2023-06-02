#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#pragma GCC diagnostic pop

#include "result.hpp"

template <typename T>
class MutexGuard {
 private:
  SemaphoreHandle_t mutex;
  T& data;

 public:
  MutexGuard(T& data, SemaphoreHandle_t mutex) : mutex(mutex), data(data) {}

  ~MutexGuard() { xSemaphoreGive(mutex); }

  T& operator*() { return this->data; }
  T* operator->() { return &this->data; }
};

template <typename T>
class Mutex {
 private:
  T data;
  SemaphoreHandle_t mutex;

 public:
  Mutex(T&& data) : data(data) { this->mutex = xSemaphoreCreateMutex(); }

  Mutex(const T& data) : data(data) { this->mutex = xSemaphoreCreateMutex(); }

  Result<MutexGuard<T>> Lock() {
    auto ret = xSemaphoreTake(this->mutex, portMAX_DELAY);
    if (ret != pdTRUE) {
      return ESP_ERR_TIMEOUT;
    }

    return Result<MutexGuard<T>>::Ok(MutexGuard<T>(this->data, this->mutex));
  }
};