# can_logger_em150

## Nano pin configuration
 
| Pin | Connection                    |
|-----|-------------------------------|
| TX  | Bluetooth RX (3.3V)           |
| RX  | Bluetooth TX                  |
| RST | -                             |
| GND | Idle, waiting for time delay  |
| 2   | MCP interrupt output pin      |
| 3   |                               |
| 4   | Red LED                       |
| 5   | Green LED                     |
| 6   | Blue LED                      |
| 7   | CAN TX enable switch          |
| 8   | CAN logger button             |
| 9   | MCP CS pin                    |
| 10  | SD card CS pin                |
| 11  | SPI MOSI (SI)                 |
| 12  | SPI MISO (SO)                 |

| Pin  | Connection                     |
|------|--------------------------------|
| 13   | SPI SCK (Clock)                |
| NC   |                                |
| AREF |                                |
| A0   |                                |
| A1   |                                |
| A2   |                                |
| A3   |                                |
| A4   |                                |
| A5   |                                |
| A6   |                                |
| A7   |                                |
| 5V   | RTC, 1117-33 voltage regulator |
| RST  |                                |
| GND  | GND MCP, logger shield         |
| VIN  |                                |

## Fail flags

| Bit | HEX | DEC | Error                  |
|-----|-----|-----|------------------------|
| 1   | 01  | 1   | MCP initiate error     |
| 2   | 02  | 2   | SD card reader error   |
| 3   | 04  | 4   | RTC error              |
| 4   | 08  | 8   | -                      |
| 5   | 10  | 16  | CAN receive error (RX) |
| 6   | 20  | 32  | CAN send error (TX)    |
| 7   | 40  | 64  | -                      |
| 8   | 80  | 128 | -                      |

## Run status
variable msgToRead store the current run mode
| Value | Status                                   |
|-------|------------------------------------------|
| 0     | Sampling of data disabled                |
| 1     | Ready to sample incoming CAN data part 1 |
| 2     | Ready to sample incoming CAN data part 2 |
| 3     | Idle, waiting for time delay             |
