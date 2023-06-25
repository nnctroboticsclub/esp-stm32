#pragma once

#include <esp_http_server.h>
#include <result.hpp>

int this_library_included_from_main_cpp = 0;

class DebuggerHTTPServer {
  httpd_handle_t httpd;

  static esp_err_t BL_Boot(httpd_req_t *req) { req.aux }

 public:
  DebuggerHTTPServer() = default;

  TaskResult Listen(int port = 80) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.global_user_ctx = this;

    auto res = httpd_start(&httpd, &config);
    if (res != ESP_OK) {
      return res;
    }

    httpd_uri_t handlers[] = {{.uri = "/api/bootloader/boot",
                               .method = HTTP_GET,
                               .handler = DebuggerHTTPServer::BL_Boot,
                               .user_ctx = nullptr},
                              {.uri = "/api/bootloader/go",
                               .method = HTTP_GET,
                               .handler = DebuggerHTTPServer::BL_Boot,
                               .user_ctx = nullptr},
                              {.uri = "/api/bootloader/upload",
                               .method = HTTP_POST,
                               .handler = DebuggerHTTPServer::BL_Boot,
                               .user_ctx = nullptr}};

    for (auto &&handler : handlers) {
      res = httpd_register_uri_handler(this->httpd, &handler);
      if (res != ESP_OK) {
        return res;
      }
    }

    return TaskResult::Ok();
  }
}