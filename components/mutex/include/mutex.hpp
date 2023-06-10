#pragma once

#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#pragma GCC diagnostic pop

#include "result.hpp"

class MutexReleaser {
 private:
  SemaphoreHandle_t mutex;

 public:
 public:
  MutexReleaser(const MutexReleaser&) = delete;
  MutexReleaser& operator=(const MutexReleaser&) = delete;

  MutexReleaser(MutexReleaser&&) = delete;
  MutexReleaser& operator=(MutexReleaser&&) = delete;

  MutexReleaser(SemaphoreHandle_t mutex) : mutex(mutex) {}

  ~MutexReleaser() { xSemaphoreGive(this->mutex); }
};

template <typename T>
class MutexGuard {
 private:
  std::shared_ptr<MutexReleaser> mutex_guard;
  T& data;

 public:
  MutexGuard(T& data, SemaphoreHandle_t mutex)
      : mutex_guard(new MutexReleaser(mutex)), data(data) {}

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