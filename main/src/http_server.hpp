#pragma once

#include <stdarg.h>
#include <esp_http_server.h>

#include <memory>
#include <vector>

#include "config/config.hpp"
#include "init/init.hpp"

namespace debug_httpd {
struct STM32State {
  std::shared_ptr<stm32::STM32> stm32;
  std::optional<stm32::session::BootLoaderSession> stm32_bl = std::nullopt;

  STM32State(uint8_t id) {
    auto stm32 = config::Config::GetSTM32(id);
    this->stm32 = stm32;
  }

  STM32State() : STM32State(config::Config::GetInstance().master.primary_s32) {}
};

class DebuggerHTTPServer {
  static constexpr const char *TAG = "Debug HTTPd";

  httpd_handle_t httpd = nullptr;

  // URI Handlers
  static esp_err_t BL_Boot(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    auto &obj = *static_cast<STM32State *>(req->user_ctx);
    if (obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "Bootloader is already launched");
      return ESP_FAIL;
    }

    auto bl = obj.stm32->EnterBootloader();
    if (!bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "Failed to enter Bootloader");
      return ESP_FAIL;
    }

    obj.stm32_bl = bl;
    httpd_resp_sendstr(req, "OK");

    return ESP_OK;
  }
  static esp_err_t BL_Upload(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    auto &obj = *static_cast<STM32State *>(req->user_ctx);
    if (!obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "No Bootloader is launched");
      return ESP_FAIL;
    }
    auto bl = *obj.stm32_bl;

    auto size = req->content_len;
    ESP_LOGI(TAG, "Uploading %d bytes", size);

    // Erasing...
    bl.Erase(stm32::driver::ErasePages(0x0800'0000, size));

    // Receive & Write Loop
    size_t read = 0;
    std::vector<uint8_t> buf(8192);  // 0x2000, 8 KB
    while (read < size) {
      auto data_size_to_read = std::min(int(size - read), 8192);
      buf.resize(data_size_to_read);

      auto ret = httpd_req_recv(req, reinterpret_cast<char *>(buf.data()),
                                data_size_to_read);

      if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
        continue;
      }
      if (ret == HTTPD_SOCK_ERR_FAIL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to receive data");
        return ESP_FAIL;
      }

      // ESP_LOGI(TAG, "Writing %d bytes", ret);
      buf.resize(ret);
      bl.WriteMemory(0x0800'0000 + read, buf);
      read += ret;
    }

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
  }
  static esp_err_t BL_Go(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    auto &obj = *static_cast<STM32State *>(req->user_ctx);
    if (!obj.stm32_bl.has_value()) {
      httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                          "No Bootloader is launched");
      return ESP_FAIL;
    }
    auto bl = *obj.stm32_bl;

    bl.Go(0x0800'0000);

    obj.stm32_bl.reset();
    obj.stm32_bl = std::nullopt;

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
  }

  static esp_err_t STM32_Reset(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    auto &obj = *static_cast<STM32State *>(req->user_ctx);

    obj.stm32->Reset();
    obj.stm32_bl = std::nullopt;

    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
  }

  static esp_err_t Reset(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, "OK");

    xTaskCreate((TaskFunction_t)([](void *args) {
                  vTaskDelay(200 / portTICK_PERIOD_MS);
                  esp_restart();
                  return;
                }),
                "Reset Thread", 0x1000, nullptr, 1, nullptr);

    return ESP_OK;
  }

  static esp_err_t NVS_Dump(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Content-Type", "application/octet-stream");

    nvs_flash_init();

    nvs_iterator_t it;
    ESP_ERROR_CHECK(nvs_entry_find(NVS_DEFAULT_PART_NAME, nullptr,
                                   nvs_type_t::NVS_TYPE_ANY, &it));

    char null_buf[8] = {0};

    std::vector<uint8_t> buf;
    while (true) {
      nvs_entry_info_t info;
      ESP_ERROR_CHECK(nvs_entry_info(it, &info));
      ESP_LOGI(TAG, "Dumping %s/%s", info.namespace_name, info.key);

      buf.emplace_back((char)strlen(info.namespace_name));
      buf.emplace_back((char)strlen(info.key));
      buf.emplace_back(info.type);

      buf.insert(buf.end(), info.namespace_name,
                 info.namespace_name + strlen(info.namespace_name));

      buf.insert(buf.end(), info.key, info.key + strlen(info.key));

      nvs_handle_t handle;
      nvs_open(info.namespace_name, NVS_READONLY, &handle);
      if (info.type == NVS_TYPE_U8 || info.type == NVS_TYPE_I8) {
        uint8_t value;
        if (info.type == NVS_TYPE_U8) {
          nvs_get_u8(handle, info.key, &value);
        } else {
          nvs_get_i8(handle, info.key, reinterpret_cast<int8_t *>(&value));
        }

        buf.emplace_back(value);
      } else if (info.type == NVS_TYPE_U16 || info.type == NVS_TYPE_I16) {
        uint16_t value;
        if (info.type == NVS_TYPE_U16) {
          nvs_get_u16(handle, info.key, &value);
        } else {
          nvs_get_i16(handle, info.key, reinterpret_cast<int16_t *>(&value));
        }

        buf.emplace_back(static_cast<char>((value >> 8) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 0) & 0xff));
      } else if (info.type == NVS_TYPE_U32 || info.type == NVS_TYPE_I32) {
        uint32_t value;
        if (info.type == NVS_TYPE_U32) {
          nvs_get_u32(handle, info.key, &value);
        } else {
          nvs_get_i32(handle, info.key, reinterpret_cast<int32_t *>(&value));
        }

        buf.emplace_back(static_cast<char>((value >> 24) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 16) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 8) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 0) & 0xff));
      } else if (info.type == NVS_TYPE_U64 || info.type == NVS_TYPE_I64) {
        uint64_t value;
        if (info.type == NVS_TYPE_U64) {
          nvs_get_u64(handle, info.key, &value);
        } else {
          nvs_get_i64(handle, info.key, reinterpret_cast<int64_t *>(&value));
        }

        buf.emplace_back(static_cast<char>((value >> 56) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 48) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 40) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 32) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 24) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 16) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 8) & 0xff));
        buf.emplace_back(static_cast<char>((value >> 0) & 0xff));
      } else if (info.type == NVS_TYPE_STR || info.type == NVS_TYPE_BLOB) {
        size_t length = 0;
        uint8_t *value;
        if (info.type == NVS_TYPE_STR) {
          nvs_get_str(handle, info.key, nullptr, &length);
          value = new uint8_t[length];
          nvs_get_str(handle, info.key, reinterpret_cast<char *>(value),
                      &length);
        } else {
          nvs_get_blob(handle, info.key, nullptr, &length);
          value = new uint8_t[length];
          nvs_get_blob(handle, info.key, static_cast<void *>(value), &length);
        }

        buf.emplace_back(static_cast<char>((length >> 24) & 0xff));
        buf.emplace_back(static_cast<char>((length >> 16) & 0xff));
        buf.emplace_back(static_cast<char>((length >> 8) & 0xff));
        buf.emplace_back(static_cast<char>(length & 0xff));

        buf.insert(buf.end(), value, value + length);

        delete[] value;
      } else {
        buf.insert(buf.end(), null_buf, null_buf + 4);
      }

      nvs_close(handle);

      if (nvs_entry_next(&it) != ESP_OK) {
        break;
      }
    }
    std::vector<uint8_t> buf2;
    for (auto c : buf) {
      if (c == '@') {
        buf2.emplace_back('@');
        buf2.emplace_back('@');
      } else if (c == '\r') {
        buf2.emplace_back('@');
        buf2.emplace_back('1');
      } else if (c == '\n') {
        buf2.emplace_back('@');
        buf2.emplace_back('2');
      } else {
        buf2.emplace_back(c);
      }
    }
    httpd_resp_send(req, (const char *)buf2.data(), buf2.size());
    ESP_LOGI(TAG, "Done Dumping");
    return ESP_OK;
  }

 public:
  DebuggerHTTPServer() = default;

  void Listen(uint16_t port = 80) {
    init::init_wifi();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = port;

    ESP_ERROR_CHECK(httpd_start(&httpd, &config));

    ESP_LOGI(TAG, "HTTPD Started on port %d", port);

    auto primary_stm32 = new STM32State();

    const httpd_uri_t handlers[] = {
        {.uri = "/api/stm32/bootloader/boot",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::BL_Boot,
         .user_ctx = static_cast<void *>(primary_stm32)},
        {.uri = "/api/stm32/bootloader/go",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::BL_Go,
         .user_ctx = static_cast<void *>(primary_stm32)},
        {.uri = "/api/stm32/bootloader/upload",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::BL_Upload,
         .user_ctx = static_cast<void *>(primary_stm32)},
        {.uri = "/api/stm32/reset",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::STM32_Reset,
         .user_ctx = static_cast<void *>(primary_stm32)},
        {.uri = "/api/reset",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::Reset,
         .user_ctx = nullptr},
        {.uri = "/api/nvs/dump",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::NVS_Dump,
         .user_ctx = nullptr},

    };

    for (auto &&handler : handlers) {
      ESP_LOGI(TAG, "Registering Endpoint: %s", handler.uri);
      ESP_ERROR_CHECK(httpd_register_uri_handler(this->httpd, &handler));
    }
  }
};
}  // namespace debug_httpd