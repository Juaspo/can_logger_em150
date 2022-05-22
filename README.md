# can_logger_em150

## Nano pin configuration
 
| Pin | Connection                    |
|-----|-------------------------------|
| TX  | Bluetooth RX (3.3V)           |
| RX  | Bluetooth TX                  |
| RST | -                             |
| GND | Idle, waiting for time delay  |
| 2   |                               |
| 3   |                               |
| 4   | Red LED                       |
| 5   | Green LED                     |
| 6   | Blue LED                      |
| 7   | CAN TX enable switch          |
| 8   | CAN logger button             |
| 9   | MCP CS pin                    |
| 10  | SD card CS pin                |
| 11  | SPI MOSI                      |
| 12  | SPI MISO                      |

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
| GND  |                                |
| VIN  |                                |

## Fail flags 
| Bit | Error                  |
|-----|------------------------|
| 1   | MCP initiate error     |
| 2   | SD card reader error   |
| 3   | RTC error              |
| 4   | -                      |
| 5   | CAN receive error (RX) |
| 6   | CAN send error (TX)    |
| 7   | -                      |
| 8   | -                      |

## Run status
variable msgToRead store the current run mode
| Value | Status                                   |
|-------|------------------------------------------|
| 0     | Sampling of data disabled                |
| 1     | Ready to sample incoming CAN data part 1 |
| 2     | Ready to sample incoming CAN data part 2 |
| 3     | Idle, waiting for time delay             |
