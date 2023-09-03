# Program Design

## STM32RC Protocol

00 Component: UI Data
01: UI DataRequest
02: Data

## config prototype

- SPIBus: bus_no mo_si mi_so clk
- UARPort: port, tx, rx, baud_rate, parity
- STM32: id, Boot0, Reset
- STM32BL: id, bus_type{UART, SPI}, bus_no
- STM32RC: id, bus_type{UART, SPI}, bus_no
- Network: NetworkProfile + id
- ServerProfile: ServerProfile + id

## nvs namespace design

### master

- spi_cnt: Number of SPI Bus
- uart_cnt: Number of UART Bus
- s32_cnt: Number of STM32 Object
- s32bl_cnt: Number of STM32 Bootloader Object
- s32rc_cnt: Number of STM32 Bootloader Object
- net_cnt: Number of Network Object
- srv_cnt: Number of Server Object
- netprof: Active Network Profile id
- srvprof: Active Server Profile id

### Other

- {spi, uart, s32, s23bl}{0,...}
