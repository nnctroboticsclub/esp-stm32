#pragma once

#include <memory>
#include <thread>

#include <data_proxy/handler.hpp>

namespace esp_stm32::data_proxy {

class ESPMaster : public Handler {
  std::shared_ptr<Bus> NewBus(Packet const& bus_id) override;

 public:
  using Handler::Handler;
};

}  // namespace esp_stm32::data_proxy