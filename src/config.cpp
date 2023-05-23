#include "config.hpp"

#define RESET GPIO_NUM_19
#define BOOT0 GPIO_NUM_21
#define led GPIO_NUM_2

namespace config {
// app::Wifi network("C0CB380B80A3", "2211560272016");
app::Wifi network("***REMOVED***", "***REMOVED***");
// app::Wifi network;

STMBootLoader loader(RESET, BOOT0, UART_NUM_1, 5, 4);

#ifdef USE_DATA_SERVER
Server server;
#endif
}  // namespace config
