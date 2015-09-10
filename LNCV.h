typedef struct {
        // name       LNCV  default  range   description
  uint16_t addr;  //    0        1  0-2048  The module address 
  uint8_t FB_addr1; //  1        0  0-255   The address of the 1st S88 module, 1st half
  uint8_t FB_addr2; //  2        0  0-255   The address of the 1st S88 module, 2nd half
  uint8_t FB_addr3; //  3        0  0-255   The address of the 3rd S88 module
  uint8_t FB_addr4; //  4        0  0-255   The address of the 4th S88 module
  uint8_t FB_addr5; //  5        0  0-255   The address of the 5th S88 module
  uint8_t FB_addr6; //  6        0  0-255   The address of the 6th S88 module
  uint8_t FB_addr7; //  7        0  0-255   The address of the 7th S88 module
  uint8_t FB_addr8; //  8        0  0-255   The address of the 8th S88 module
  uint8_t NoS88;    //  9        1  0-7     Number of S88 modules connected to the S88 bus
} LNCV_LNS88_t;

