#pragma once

#include <nvs.h>
#include <esp_log.h>
#include <memory>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <driver/gpio.h>

namespace nvs {
class Namespace {
  static constexpr const char* TAG = "nvs::Namespace";

 public:
  nvs_handle_t handle_;
  const char* ns;

 public:
  Namespace(const char* ns) {
    this->ns = new char[strlen(ns) + 1];
    strcpy((char*)this->ns, ns);

    ESP_LOGI(TAG, "open %s", ns);
    auto err = nvs_open(ns, NVS_READWRITE, &handle_);
    if (err == ESP_ERR_NVS_NOT_INITIALIZED) {
      nvs_flash_init();
      err = nvs_open(ns, NVS_READWRITE, &handle_);
    }
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "NVS open failed: %s[%d] %s", esp_err_to_name(err), err,
               ns);
      esp_system_abort("NVS Open Failed nvs_proxy.hpp:L20");
    }
  }
  ~Namespace() {
    this->Commit();
    ESP_LOGI(TAG, "close %s", this->ns);
    nvs_close(handle_);
  }

  void Commit() {
    ESP_LOGI(TAG, "commit %s", ns);
    auto ret = nvs_commit(handle_);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "NVS commit failed: %s[%d] %s", esp_err_to_name(ret), ret,
               ns);
      esp_system_abort("NVS Commit Failed nvs_proxy.hpp:L39");
    }
  }
};
class _Proxy {
 protected:
  static constexpr const char* TAG = "nvs::Proxy";
  std::shared_ptr<nvs::Namespace> ns_;
  const char* key_;

 public:
  _Proxy(nvs::Namespace& ns_handle, const char* key)
      : ns_(&ns_handle), key_(key) {}
  _Proxy(nvs::Namespace* ns_handle, const char* key)
      : ns_(ns_handle), key_(key) {}
  _Proxy() = delete;
  ~_Proxy() {}
};

// real Proxy objects
template <typename T>
class Proxy {};

template <>
class Proxy<uint8_t> : public _Proxy {
 public:
  using _Proxy::_Proxy;

  operator uint8_t() {
    uint8_t value;
    auto ret = nvs_get_u8(ns_->handle_, key_, &value);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      *this = 0;
      value = 0;
    }
    return value;
  }

  Proxy& operator=(uint8_t value) {
    auto ret = nvs_set_u8(ns_->handle_, key_, value);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      esp_system_abort("NVS Set Failed nvs_proxy.hpp:L80");
    }
    return *this;
  }
};

template <>
class Proxy<uint16_t> : public _Proxy {
 public:
  using _Proxy::_Proxy;

  operator uint16_t() {
    uint16_t value;
    auto ret = nvs_get_u16(ns_->handle_, key_, &value);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      *this = 0;
      value = 0;
    }
    return value;
  }

  Proxy& operator=(uint16_t value) {
    nvs_set_u16(ns_->handle_, key_, value);
    return *this;
  }
};

template <>
class Proxy<uint32_t> : public _Proxy {
 public:
  using _Proxy::_Proxy;

  operator uint32_t() {
    uint32_t value;
    auto ret = nvs_get_u32(ns_->handle_, key_, &value);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      *this = 0;
      value = 0;
    }
    return value;
  }

  Proxy& operator=(uint32_t value) {
    auto ret = nvs_set_u32(ns_->handle_, key_, value);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      esp_system_abort("NVS Set Failed nvs_proxy.hpp:L130");
    }
    return *this;
  }
};

template <size_t N>
class Proxy<char[N]> : public _Proxy {
 public:
  using _Proxy::_Proxy;

  operator char*() {
    size_t len;
    auto ret = nvs_get_str(ns_->handle_, key_, NULL, &len);
    if (ret != ESP_OK) {
      static char buf[20]{};
      ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      *this = buf;
      len = 20;
    }
    if (len > N) {
      ESP_LOGE(TAG, "NVS string length is too long: %u > %u", len, N);
      esp_system_abort("NVS String Length Too Long nvs_proxy.hpp:L146");
    }

    char* value = new char[N];
    ret = nvs_get_str(ns_->handle_, key_, value, &len);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
               key_);
      esp_system_abort("NVS Get Failed nvs_proxy.hpp:L154");
    }
    return value;
  }

  Proxy& operator=(const char* value) {
    nvs_set_str(ns_->handle_, key_, value);
    return *this;
  }
};

template <>
class Proxy<gpio_num_t> : public Proxy<uint8_t> {
 public:
  using Proxy<uint8_t>::Proxy;

  operator gpio_num_t() {
    return (gpio_num_t)Proxy<uint8_t>::operator uint8_t();
  }

  Proxy& operator=(gpio_num_t value) {
    Proxy<uint8_t>::operator=(value);
    return *this;
  }
};

}  // namespace nvs
