#include <stm32/session/stm32.hpp>
#include <stm32/session/bootloader.hpp>

namespace stm32::session {

void Session::SetModeBootLoader() {
  this->boot0_.set_high();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
void Session::UnsetModeBootLoader() {
  this->boot0_.set_low();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
//* Public
Session::Session(std::shared_ptr<raw_driver::RawDriverBase> raw_bl_driver,
                 idf::GPIONum boot0, idf::GPIONum reset)
    : raw_bl_driver_(raw_bl_driver), boot0_(boot0), reset_(reset) {
  this->boot0_.set_low();
  this->reset_.set_high();
}
void Session::Reset() {
  this->reset_.set_low();
  vTaskDelay(50 / portTICK_PERIOD_MS);

  this->reset_.set_high();
}
BootLoaderSession Session::EnterBL() {
  return BootLoaderSession(this->raw_bl_driver_,
                           std::make_shared<Session>(*this));
}

std::optional<BootLoaderSession> Session::TryEnterBL(int tries) {
  for (int i = 0; i < tries; i++) {
    ESP_LOGI(TAG, "Try %d", i);
    try {
      return this->EnterBL();
    } catch (raw_driver::ConnectionDisrupted &) {
      ESP_LOGW(TAG, "ConnectionDisrupted");
    }
  }

  return std::nullopt;
}
}  // namespace stm32::session
