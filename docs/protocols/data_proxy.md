# Data Proxy

## Format

| Offset | Size | Name        | Description             |
| ------ | ---- | ----------- | ----------------------- |
| 0x00   | 1    | Command     | Command                 |
| 0x01   | 1    | Bus ID      | Bus ID                  |
| 0x02   | 1    | Port ID     | Port ID                 |
| 0x03   | 1    | optional    | Reserved for future use |
| 0x04   | 4    | Data Length | Data Length             |
| 0x08   | n    | Data        | Data                    |

### Command

| Type      | Scope  | Dir | Op  | Name           | Data             |
| --------- | ------ | --- | --- | -------------- | ---------------- |
| Session   | BusID  | Req | 0   | Close          | None             |
| ^         | ^      | ^   | 1   | New I2C        | sda, scl         |
| ^         | ^      | ^   | 2   | New SPI        | mosi, miso, sclk |
| ^         | ^      | ^   | 3   | Unassigned     | None             |
| ^         | ^      | Res | 0   | Created Bus    | None             |
| ^         | ^      | ^   | 1   | Closed Bus     | None             |
| ^         | ^      | ^   | 2   | Unassigned     | None             |
| ^         | ^      | ^   | 3   | ^              | None             |
| ^         | PortID | Req | 0   | Close Port     | None             |
| ^         | ^      | ^   | 1   | Create         | None             |
| ^         | ^      | ^   | 2   | Unassigned     | None             |
| ^         | ^      | ^   | 3   | ^              | None             |
| ^         | ^      | Res | 0   | Created Port   | None             |
| ^         | ^      | ^   | 1   | Closed Port    | None             |
| ^         | ^      | ^   | 2   | Error          | Error number     |
| ^         | ^      | ^   | 3   | Unassigned     | None             |
| ReadWrite | PortID | Req | 0   | Write          | data             |
| ^         | ^      | ^   | 1   | Read           | length           |
| ^         | ^      | ^   | 2   | AvailableDatas | None             |
| ^         | ^      | ^   | 3   | Unassigned     | None             |
| ^         | ^      | Res | 0   | Data           | data             |
| ^         | ^      | ^   | 1   | Length         | length           |
| ^         | ^      | ^   | 2   | Unassigned     | None             |
| ^         | ^      | ^   | 3   | ^              | None             |
| List      | Global | Req | 0   | List Bus       | None             |
| ^         | ^      | Res | 0   | Bus List       | u8[]             |
| ^         | BusId  | Req | 0   | List Port      | None             |
| ^         | ^      | Res | 0   | Port List      | u16[]            |

#### Command Byte

| Bit | Description      |
| --- | ---------------- |
| 0-2 | Kind             |
| 3-4 | Scope, Direction |
| 5-7 | Op               |

### Bus ID

| 1-2 | 3-8 | Name    |
| --- | --- | ------- |
| 0   | ID  | SPI xx  |
| 1   | ID  | I2C xx  |
| 2   | ID  | UART xx |
| 3   | ID  | Pipe    |

### Port ID

| BusType | Value    | Name           |
| ------- | -------- | -------------- |
| SPI     | xxxxxxxx | SPI(cs = xx)   |
| I2C     | xxxxxxxx | I2c(addr = xx) |
| UART    | 00000000 | UART Bus       |
