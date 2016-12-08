|   CV | Name         | (Default) value | Range | Access | Description                                                         |
|-----:|--------------|-----------------|-------|--------|---------------------------------------------------------------------|
|    1 | Address      | 127             | 0-255 | R/W    | Module address                                                      |
|    2 |              |                 |       |        |                                                                     |
|    3 |              |                 |       |        |                                                                     |
|    4 |              |                 |       |        |                                                                     |
|    5 | Max Modules  | 31              | 0-255 | RO     | Maximum number of 16 bit S88 modules that can be handled on the bus |
|    6 | No Modules   | 1               | 0-CV5 | R/W    | Number of 16 input S88 modules connected                            |
|    7 | Manufacturer | 7               |       | RO     | 7 = DIY                                                             |
|    8 | Version      | 1               |       | RO     | Version of the firmware                                             |
|    9 | Speed        | 20              | 1-255 | R/W    | Speed of the S88 bus                                                |
| 1024 | FreeRam      |                 |       | RO     | DEBUG: Amount of free RAM                                           |
| 1025 | bss_start    |                 |       | RO     |                                                                     |
| 1026 | bss_end      |                 |       | RO     |                                                                     |
| 1027 | SP           |                 |       | RO     |                                                                     |
| 1028 | RxPackets    |                 |       | RO     | DEBUG: Number of received Loconet packets                           |
| 1029 | RxErrors     |                 |       | RO     | DEBUG: Number of failed to receive Loconet packets                  |
| 1030 | TxPackets    |                 |       | RO     | DEBUG: Number of send Loconet packets                               |
| 1031 | TxErrors     |                 |       | RO     | DEBUG: Number of failed to send Loconet packets                     |
| 1032 | Collisions   |                 |       | RO     | DEBUG: Number of collisions on the Loconet bus                      |