#pragma once

#include <memory>
#include <thread>

#include <data_proxy/link.hpp>
#include <data_proxy/bus.hpp>

namespace esp_stm32::data_proxy {
namespace master {
class Master {
  class Impl;
  std::shared_ptr<Impl> impl;

 public:
  explicit Master(Link link);

  std::shared_ptr<Bus> GetBus(BusID bus_id);

  std::thread Start();
};
}  // namespace master
using Master = master::Master;
}  // namespace esp_stm32::data_proxy