#pragma once

#include <string.h>
#include <nvs.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <utility>
#include <memory>
#include <vector>
#include <string>

namespace nvs {
namespace api {
constexpr const char* TAG = "nvs::api";

class NVSError : public std::runtime_error {
 public:
  NVSError() : std::runtime_error("NVS Error") {}
};

inline nvs_handle_t Open(std::string const& name, nvs_open_mode_t open_mode) {
  nvs_handle_t handle;

  auto result = nvs_open(name.c_str(), open_mode, &handle);
  if (result == ESP_ERR_NVS_NOT_INITIALIZED) {
    nvs_flash_init();
    result = nvs_open(name.c_str(), open_mode, &handle);
  }
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "NVS open failed: %s[%d] %s", esp_err_to_name(result), result,
             name.c_str());
    throw NVSError();
  }

  return handle;
}

inline void Close(nvs_handle_t handle) { nvs_close(handle); }

inline void Commit(nvs_handle_t handle) {
  auto ret = nvs_commit(handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS commit failed: %s[%d]", esp_err_to_name(ret), ret);
    throw NVSError();
  }
}

template <typename T>
T Get(nvs_handle_t, std::string const&) {
  using T2 = T::NonExistType;  // SFINAE
  (void)T2(0);  // Avoid 'typedef xxx locally defined but not used' error
}

template <>
inline uint8_t Get<uint8_t>(nvs_handle_t handle, std::string const& key) {
  uint8_t value;
  if (auto ret = nvs_get_u8(handle, key.c_str(), &value); ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    return 0;
  }
  return value;
}

template <>
inline uint16_t Get<uint16_t>(nvs_handle_t handle, std::string const& key) {
  uint16_t value;
  if (auto ret = nvs_get_u16(handle, key.c_str(), &value); ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    return 0;
  }
  return value;
}

template <>
inline uint32_t Get<uint32_t>(nvs_handle_t handle, std::string const& key) {
  uint32_t value;
  if (auto ret = nvs_get_u32(handle, key.c_str(), &value); ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    return 0;
  }
  return value;
}

template <>
inline std::string Get<std::string>(nvs_handle_t handle,
                                    std::string const& key) {
  size_t length = 0;
  if (auto ret = nvs_get_str(handle, key.c_str(), nullptr, &length);
      ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    return "";
  }

  auto value = new char[length];
  if (auto ret = nvs_get_str(handle, key.c_str(), value, &length);
      ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS get failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    return "";
  }

  std::string result(value);
  delete[] value;

  return result;
}

template <typename T>
void Set(nvs_handle_t, std::string const&, T const&) {
  using T2 = T::NonExistType;  // SFINAE
  (void)T2(0);  // Avoid 'typedef xxx locally defined but not used' error
}

template <>
inline void Set<uint8_t>(nvs_handle_t handle, std::string const& key,
                         uint8_t const& value) {
  auto ret = nvs_set_u8(handle, key.c_str(), value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    throw NVSError();
  }
}

template <>
inline void Set<uint16_t>(nvs_handle_t handle, std::string const& key,
                          uint16_t const& value) {
  auto ret = nvs_set_u16(handle, key.c_str(), value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    throw NVSError();
  }
}

template <>
inline void Set<uint32_t>(nvs_handle_t handle, std::string const& key,
                          uint32_t const& value) {
  auto ret = nvs_set_u32(handle, key.c_str(), value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    throw NVSError();
  }
}

template <>
inline void Set<std::string>(nvs_handle_t handle, std::string const& key,
                             std::string const& value) {
  auto zero_terminated = new char[value.length() + 1];
  snprintf(zero_terminated, value.length() + 1, "%s", value.c_str());

  if (auto ret = nvs_set_str(handle, key.c_str(), zero_terminated);
      ret != ESP_OK) {
    ESP_LOGE(TAG, "NVS set failed: %s[%d] %s", esp_err_to_name(ret), ret,
             key.c_str());
    throw NVSError();
  }

  delete[] zero_terminated;
}
}  // namespace api

// Alias Proxy (Threats T as Base)
template <typename T>
class AliasProxyTable;

template <>
class AliasProxyTable<gpio_num_t> {
  using type = uint8_t;
};

template <>
class AliasProxyTable<bool> {
  using type = uint8_t;
};

template <typename T>
using AliasProxyBaseType = typename AliasProxyTable<T>::type;

namespace detail {

class NamespaceHandle {
  nvs_handle_t handle;

 public:
  explicit NamespaceHandle(std::string const& ns) {
    this->handle = api::Open(ns, NVS_READWRITE);
  }

  NamespaceHandle(NamespaceHandle const&) = delete;
  NamespaceHandle& operator=(NamespaceHandle const&) = delete;

  NamespaceHandle(NamespaceHandle&& other) noexcept
      : handle(std::exchange(other.handle, 0)) {}

  NamespaceHandle& operator=(NamespaceHandle&& other) noexcept {
    this->handle = std::exchange(other.handle, 0);
    return *this;
  }

  ~NamespaceHandle() { api::Close(this->handle); }

  explicit operator nvs_handle_t() const { return this->handle; }
};

class Namespace {
  static constexpr const char* TAG = "nvs::Namespace";

  NamespaceHandle handle_;
  std::string ns;
  bool dirty = false;

  void Commit() const {
    api::Commit(static_cast<nvs_handle_t>(this->handle_));
    ESP_LOGI(TAG, "NVS Commited: %s", this->ns.c_str());
  }

 public:
  Namespace() = delete;

  Namespace(Namespace const&) = delete;
  Namespace& operator=(Namespace const&) = delete;

  explicit Namespace(std::string const& ns) : handle_(ns), ns(ns) {}

  ~Namespace() {
    if (this->dirty) this->Commit();
    api::Close(static_cast<nvs_handle_t>(this->handle_));
  }

  template <typename T>
  T Get(std::string const& key) const {
    return api::Get<T>(static_cast<nvs_handle_t>(this->handle_), key);
  }

  template <typename T>
  void Set(std::string const& key, T value) const {
    this->dirty = true;
    api::Set<T>(static_cast<nvs_handle_t>(this->handle_), key, value);
  }
};

class SharedNamespace {
 private:
  std::shared_ptr<Namespace> ns;

 public:
  explicit SharedNamespace(const char* ns)
      : ns(std::make_shared<Namespace>(ns)) {}

  explicit SharedNamespace(std::string const& ns)
      : ns(std::make_shared<Namespace>(ns)) {}

  explicit SharedNamespace(SharedNamespace const* other) : ns(other->ns) {}

  Namespace* operator->() { return this->ns.get(); }
};

// Direct Proxy (Use nvs::api's Get/Set)
template <typename T>
concept DirectProxySatisfy = requires {
  api::Get<T>(nvs_handle_t{}, std::string{});
  api::Set<T>(nvs_handle_t{}, std::string{}, T{});
};

template <DirectProxySatisfy T>
class DirectProxy {
 private:
  static constexpr const char* TAG = "nvs::Proxy";
  SharedNamespace ns_;
  std::string key_;

 public:
  DirectProxy(SharedNamespace ns_handle, std::string const& key)
      : ns_(ns_handle), key_(key) {}
  DirectProxy(SharedNamespace const* ns_handle, std::string const& key)
      : ns_(ns_handle), key_(key) {}

  DirectProxy() = delete;
  ~DirectProxy() = default;

  void Commit() { this->ns_->Commit(); }

  T Get() { return this->ns_->Get<T>(key_); }
  void Set(T const& value) { this->ns_->Set<T>(key_, value); }

  explicit operator T() { return this->Get(); }

  DirectProxy& operator=(T const& value) {
    this->Set(value);
    return *this;
  }
};

template <typename T>
concept AliasProxyTableExist = requires { typename AliasProxyTable<T>::type; };

template <AliasProxyTableExist T>
class AliasProxy : public DirectProxy<AliasProxyBaseType<T>> {
  using Base = AliasProxyBaseType<T>;

 public:
  using DirectProxy<Base>::DirectProxy;

  explicit operator T() { return (T)DirectProxy<Base>::operator Base(); }

  AliasProxy& operator=(T value) {
    DirectProxy<Base>::operator=(value);
    return *this;
  }
};

template <DirectProxySatisfy T>
auto DummyFunc(T) -> DirectProxy<T>;

template <AliasProxyTableExist T>
auto DummyFunc(T) -> AliasProxy<T>;

template <typename T>
using Proxy = decltype(DummyFunc(std::declval<T>()));
}  // namespace detail

using Namespace = detail::SharedNamespace;

template <typename T>
using Proxy = detail::Proxy<T>;

template <std::derived_from<Namespace> T>
std::vector<T> LoadNamespaces(std::string const& group_name, size_t count) {
  std::vector<T> result;
  for (size_t i = 0; i < count; i++) {
    result.push_back(T(group_name + std::to_string(i)));
  }
  return result;
}

void DumpNVS();

}  // namespace nvs
