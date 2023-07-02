#pragma once

#include <memory>
#include <stdexcept>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#pragma GCC diagnostic pop

namespace async {
class MutexReleaser {
 private:
  SemaphoreHandle_t mutex;

 public:
  explicit MutexReleaser(SemaphoreHandle_t mutex) : mutex(mutex) {}
  ~MutexReleaser() { xSemaphoreGive(this->mutex); }

  MutexReleaser(const MutexReleaser&) = delete;
  MutexReleaser& operator=(const MutexReleaser&) = delete;
};

template <typename T>
class MutexGuard {
 private:
  std::shared_ptr<MutexReleaser> mutex_guard;
  T& data;

 public:
  MutexGuard(T& data, SemaphoreHandle_t mutex) : data(data) {
    this->mutex_guard = std::make_shared<MutexReleaser>(mutex);
  }

  T& operator*() { return this->data; }
  T* operator->() { return &this->data; }
};

class FailedToTakeMutex : public std::runtime_error {
 public:
  FailedToTakeMutex() : std::runtime_error("Failed to take mutex") {}
};

template <typename T>
class Mutex {
 private:
  T data;
  SemaphoreHandle_t mutex;

 public:
  explicit Mutex(T&& data) : data(std::move(data)) {
    this->mutex = xSemaphoreCreateMutex();
  }

  explicit Mutex(const T& data) : data(data) {
    this->mutex = xSemaphoreCreateMutex();
  }

  MutexGuard<T> Lock() {
    auto ret = xSemaphoreTake(this->mutex, portMAX_DELAY);
    if (ret != pdTRUE) {
      throw FailedToTakeMutex();
    }

    return MutexGuard<T>(this->data, this->mutex);
  }
};
}  // namespace async