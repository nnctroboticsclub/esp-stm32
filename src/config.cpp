#include "config.hpp"

namespace config {
// app::Wifi network("C0CB380B80A3", "2211560272016");
// app::Wifi network("***REMOVED***", "***REMOVED***");
// app::Wifi network;
simple_serial::Rx rx(GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_27);
simple_serial::Tx tx(GPIO_NUM_2, GPIO_NUM_5, GPIO_NUM_4);

#ifdef USE_DATA_SERVER
Server server;
#endif
}  // namespace config
