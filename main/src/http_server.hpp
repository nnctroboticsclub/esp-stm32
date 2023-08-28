#pragma once

#include <stdarg.h>
#include <esp_http_server.h>

#include <memory>
#include <vector>

#include "config/config.hpp"

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

 public:
  DebuggerHTTPServer() = default;

  void Listen(uint16_t port = 80) {
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
         .user_ctx = static_cast<void *>(primary_stm32),
         .is_websocket = false,
         .handle_ws_control_frames = false,
         .supported_subprotocol = nullptr},
        {.uri = "/api/stm32/bootloader/go",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::BL_Go,
         .user_ctx = static_cast<void *>(primary_stm32),
         .is_websocket = false,
         .handle_ws_control_frames = false,
         .supported_subprotocol = nullptr},
        {.uri = "/api/stm32/bootloader/upload",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::BL_Upload,
         .user_ctx = static_cast<void *>(primary_stm32),
         .is_websocket = false,
         .handle_ws_control_frames = false,
         .supported_subprotocol = nullptr},
        {.uri = "/api/stm32/reset",
         .method = HTTP_POST,
         .handler = DebuggerHTTPServer::STM32_Reset,
         .user_ctx = static_cast<void *>(primary_stm32),
         .is_websocket = false,
         .handle_ws_control_frames = false,
         .supported_subprotocol = nullptr},

    };

    for (auto &&handler : handlers) {
      ESP_LOGI(TAG, "Registering Endpoint: %s", handler.uri);
      ESP_ERROR_CHECK(httpd_register_uri_handler(this->httpd, &handler));
    }
  }
};
}  // namespace debug_httpd