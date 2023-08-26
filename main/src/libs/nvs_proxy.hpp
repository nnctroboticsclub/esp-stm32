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
  return T2(0);  // Avoid 'typedef xxx locally defined but not used' error and
                 // no return statement in function returning non-void error
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

template <typename T>
struct DirectProxySatisfy : std::false_type {};

template <>
struct DirectProxySatisfy<uint8_t> : std::true_type {};

template <>
struct DirectProxySatisfy<uint16_t> : std::true_type {};

template <>
struct DirectProxySatisfy<uint32_t> : std::true_type {};

template <>
struct DirectProxySatisfy<std::string> : std::true_type {};

}  // namespace api

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

  const NamespaceHandle handle_;
  const std::string ns;
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
  void Set(std::string const& key, T value) {
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

  Namespace* operator->() const { return this->ns.get(); }
};

// Direct Proxy (Use nvs::api's Get/Set)
template <typename T>
concept DirectProxySatisfy = api::DirectProxySatisfy<T>::value;

template <DirectProxySatisfy T>
class DirectProxy {
 private:
  static constexpr const char* TAG = "nvs::Proxy";
  SharedNamespace ns_;
  std::string key_;
  T buffer;

 public:
  DirectProxy(SharedNamespace ns_handle, std::string const& key)
      : ns_(ns_handle), key_(key) {}
  DirectProxy(SharedNamespace const* ns_handle, std::string const& key)
      : ns_(ns_handle), key_(key) {}

  DirectProxy() = delete;
  ~DirectProxy() = default;

  void Commit() { this->ns_->Commit(); }

  T Get() const { return this->ns_->Get<T>(key_); }
  void Set(T const& value) { this->ns_->Set<T>(key_, value); }

  operator T() const { return this->Get(); }

  DirectProxy& operator=(T const& value) {
    this->Set(value);
    return *this;
  }
};

template <typename T, DirectProxySatisfy Base>
class AliasProxy : public DirectProxy<Base> {
 public:
  using DirectProxy<Base>::DirectProxy;

  T Get() const { return (T)DirectProxy<Base>::Get(); }
  void Set(T value) { DirectProxy<Base>::Set((Base)value); }

  explicit operator T() const { return (T)DirectProxy<Base>::operator Base(); }

  AliasProxy& operator=(T value) {
    DirectProxy<Base>::operator=(static_cast<Base>(value));
    return *this;
  }
};

}  // namespace detail

using Namespace = detail::SharedNamespace;

template <typename T>
struct AliasProxyTable {};

template <>
struct AliasProxyTable<gpio_num_t> {
  using type = uint8_t;
};
template <>
struct AliasProxyTable<bool> {
  using type = uint8_t;
};

template <typename T>
concept AliasProxySatisfy = requires { typename AliasProxyTable<T>::type; };

template <detail::DirectProxySatisfy T>
auto proxy_guesser(T) -> detail::DirectProxy<T>;

template <AliasProxySatisfy T>
auto proxy_guesser(T)
    -> detail::AliasProxy<T, typename AliasProxyTable<T>::type>;

template <typename T>
using Proxy = decltype(proxy_guesser(std::declval<T>()));

// Ref: https://qiita.com/TwilightUncle/items/aadc9f60f857e1ab7031
template <int Size>
struct CeString {
  static constexpr int length = Size - 1;

  // 文字列リテラルより推論を行うためのコンストラクタ
  constexpr CeString(const char (&s_literal)[Size]) {
    for (int i = 0; i < Size; i++) buf[i] = s_literal[i];
    buf[length] = '\0';
  }

  constexpr operator std::string_view() const { return buf; }
  constexpr operator std::string() const { return buf; }

  // 文字列格納領域
  char buf[Size];
};

template <std::derived_from<Namespace> T, CeString prefix>
class Namespaces {
  using iterator = typename std::vector<T>::iterator;
  std::vector<T> namespaces;

 public:
  Namespaces(size_t namespaces) {
    for (size_t i = 1; i <= namespaces; i++) {
      this->New();
    }
  }

  T& operator[](size_t index) { return this->namespaces[index]; }
  size_t Size() const { return this->namespaces.size(); }

  iterator begin() { return this->namespaces.begin(); }
  iterator end() { return this->namespaces.end(); }

  T& New() {
    this->namespaces.push_back(
        T(std::string(prefix) + std::to_string(this->namespaces.size() + 1)));
    return this->namespaces.back();
  }
};

void DumpNVS();

}  // namespace nvs
