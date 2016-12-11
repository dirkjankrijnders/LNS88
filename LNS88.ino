#include <LocoNet.h>
#include <S88.h>
#include "LNCV.h"
#define DEBUG(x) 
/* Include the CV handling and the pin functionalities */
#include "decoder_conf.h"
#include "cvaccess.h"

decoder_conf_t EEMEM _CV = {
#include "default_conf.h"
};

//#include "LNCV_data.h"

#define LOCONET_TX_PIN 6
#define LNCV_COUNT 10

// Item Number (Art.-Nr.): 50010
#define ARTNR 10003

void HandleS88(S88_t* S88);
void dumpPacket(UhlenbrockMsg & ub); 
//uint16_t moduleAddr;
//uint16_t lncv[LNCV_COUNT];

lnMsg *LnPacket;

LocoNetCVClass lnCV;
uint8_t* lncv;

int programmingMode;

S88_t S88;
#define MAX 31


extern int __bss_start, __bss_end;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
};

void setup(){

	SetupS88Hardware(&S88);
  SetNoModules(&S88, read_cv(&_CV, 6), 0);
	uint16_t clk = read_cv(&_CV, 9);
	SetClock(&S88, &clk, false);
	S88.Config.activeData = 0;
	LocoNet.init();
	sei();
	StartS88Read(&S88, FULL);
	LocoNet.reportSensor(654, 1);
}
void loop(){
// put your main code here, to run repeatedly:
		if (IsReady(&S88) && !programmingMode) {
				////Serial.println("Ready");
			HandleS88(&S88);
			SwapAndClearS88Data(&S88);
			StartS88Read(&S88, FULL);
		};
  		LnPacket = LocoNet.receive();
		if (LnPacket) {
				lnCV.processLNCVMessage(LnPacket);
		}
}

void HandleS88(S88_t* S88) {
	uint8_t module = 0;
	//uint8_t half = 0;
	uint8_t current_buffer = S88->Config.activeData;
	uint8_t other_buffer = (S88->Config.activeData == 0) ? 1 : 0;
	for (; module < S88->State.maxModules; module++) {
		if (S88->Config.data[other_buffer][module] != S88->Config.data[current_buffer][module]) {
      uint16_t bit = __builtin_ctzl(S88->Config.data[other_buffer][module] ^ S88->Config.data[current_buffer][module]) + 1;
      uint8_t state = ((S88->Config.data[current_buffer][module] && ((uint16_t)1 << bit)) == 1);
			LocoNet.reportSensor((read_cv(&_CV, 1) *16)+bit, state);
			//Serial.print("LN Sensor addr: ");
			//Serial.println(lnconfig.addr + lncv[1+(module*2)]);
			//Serial.print(" Old value: " );
			//Serial.println(S88->Config.data[other_buffer][module]);
			//Serial.print(" New value: " );
			//Serial.println(S88->Config.data[current_buffer][module]);
		}
	}
}

void dumpPacket(UhlenbrockMsg & ub) {
#ifdef DEBUG_OUTPUT
  Serial.print(" PKT: ");
  Serial.print(ub.command);
  Serial.print(" ");
  Serial.print(ub.mesg_size, HEX);
  Serial.print(" ");
  Serial.print(ub.SRC, HEX);
  Serial.print(" ");
  Serial.print(ub.DSTL, HEX);
  Serial.print(" ");
  Serial.print(ub.DSTH, HEX);
  Serial.print(" ");
  Serial.print(ub.ReqId, HEX);
  Serial.print(" ");
  Serial.print(ub.PXCT1, HEX);
  Serial.print(" ");
  for (int i(0); i < 8; ++i) {
    Serial.print(ub.payload.D[i], HEX);
    Serial.print(" ");
  }
  Serial.write("\n");
#endif
}

/**
   * Notifies the code on the reception of a read request.
   * Note that without application knowledge (i.e., art.-nr., module address
   * and "Programming Mode" state), it is not possible to distinguish
   * a read request from a programming start request message.
   */
int8_t notifyLNCVread(uint16_t ArtNr, uint16_t lncvAddress, uint16_t,
    uint16_t & lncvValue) {

  DEBUG("Enter notifyLNCVread(");
  DEBUG(ArtNr);
  DEBUG(", ");
  DEBUG(lncvAddress);
  DEBUG(", ");
  DEBUG(", ");
  DEBUG(lncvValue);
  DEBUG(")\n");
  
  // Step 1: Can this be addressed to me?
  // All ReadRequests contain the ARTNR. For starting programming, we do not accept the broadcast address.
  if (programmingMode) {
    if (ArtNr == ARTNR) {
    if (lncvAddress == 5){
      lncvValue = MAX;
      return LNCV_LACK_OK;
    } else if (lncvAddress < 10) {
        lncvValue = read_cv(&_CV, lncvAddress);

        DEBUG("\nEeprom address: ");
        DEBUG(((uint16_t)&(_CV.address)+cv2address(lncvAddress)));
        DEBUG(" LNCV Value: ");
        DEBUG(lncvValue);
        DEBUG("\n");

        return LNCV_LACK_OK;
      } else if (lncvAddress == 1024) {
        lncvValue = freeRam();
        return LNCV_LACK_OK;
      } else if (lncvAddress == 1025) {
        lncvValue = __bss_start;
        return LNCV_LACK_OK;
      } else if (lncvAddress == 1026) {
        lncvValue = __bss_end;
        return LNCV_LACK_OK;
      } else if (lncvAddress == 1027) {
        lncvValue = SP;
        return LNCV_LACK_OK;
      } else if ((lncvAddress > 1027) && (lncvAddress < 1033)){
        LnBufStats* stats = LocoNet.getStats();
        switch (lncvAddress) {
          case 1028:
          lncvValue = stats->RxPackets;
          break;
          case 1029:
          lncvValue = stats->RxErrors;
          break;
          case 1030:
          lncvValue = stats->TxPackets;
          break;
          case 1031:
          lncvValue = stats->TxErrors;
          break;
          case 1032:
          lncvValue = stats->Collisions;
        }
        return LNCV_LACK_OK;
      } else {
        // Invalid LNCV address, request a NAXK
        return LNCV_LACK_ERROR_UNSUPPORTED;
      }
    } else {
      DEBUG("ArtNr invalid.\n");
      return -1;
    }
  } else {
    DEBUG("Ignoring Request.\n");
    return -1;
  }
}

int8_t notifyLNCVprogrammingStart(uint16_t & ArtNr, uint16_t & ModuleAddress) {
  // Enter programming mode. If we already are in programming mode,
  // we simply send a response and nothing else happens.
  uint16_t MyModuleAddress = eeprom_read_byte(&_CV.address);

  DEBUG(ArtNr);
  DEBUG(ModuleAddress);
  DEBUG(MyModuleAddress);


  if (ArtNr == ARTNR) {
    if (ModuleAddress == MyModuleAddress) {
      programmingMode = true;
      DEBUG("Programming started");
      return LNCV_LACK_OK;
    } else if (ModuleAddress == 0xFFFF) {
      ModuleAddress = eeprom_read_byte(&_CV.address);
      return LNCV_LACK_OK;
    }
  }
  return -1;
}

  /**
   * Notifies the code on the reception of a write request
   */
int8_t notifyLNCVwrite(uint16_t ArtNr, uint16_t lncvAddress,
    uint16_t lncvValue) {
  //  dumpPacket(ub);
  if (!programmingMode) {
    return -1;
  }

  DEBUG("Enter notifyLNCVwrite(");
  DEBUG(ArtNr);
  DEBUG(", ");
  DEBUG(lncvAddress);
  DEBUG(", ");
  DEBUG(", ");
  DEBUG(lncvValue);
  DEBUG(")\n");

  if (ArtNr == ARTNR) {

    if (lncvAddress < 320) {
      DEBUG(cv2address(lncvAddress));
      DEBUG(bytesizeCV(lncvAddress));
      DEBUG((uint8_t)lncvValue);
      write_cv(&_CV, lncvAddress, lncvValue);
      return LNCV_LACK_OK;
    }
    else {
      return LNCV_LACK_ERROR_UNSUPPORTED;
    }
  }
  else {

    DEBUG("Artnr Invalid.\n");
    return -1;
  }
}

void commitLNCVUpdate() {
   // Reset the decoder to reread the configuration
  //asm volatile ("  jmp 0");
}
  /**
   * Notifies the code on the reception of a request to end programming mode
   */
void notifyLNCVprogrammingStop(uint16_t ArtNr, uint16_t ModuleAddress) {
  DEBUG("notifyLNCVprogrammingStop ");
  if (programmingMode) {
    if (ArtNr == ARTNR && ModuleAddress == eeprom_read_byte(&_CV.address)) {
      programmingMode = false;
      DEBUG("End Programing Mode.\n");
      commitLNCVUpdate();
    }
    else {
      if (ArtNr != ARTNR) {
        DEBUG("Wrong Artnr.\n");
        return;
      }
      if (ModuleAddress != eeprom_read_byte(&_CV.address)) {
        DEBUG("Wrong Module Address.\n");
        return;
      }
    }
  }
  else {
    DEBUG("Ignoring Request.\n");
  }
}

int8_t notifyLNCVdiscover( uint16_t & ArtNr, uint16_t & ModuleAddress ) {
  uint16_t MyModuleAddress = eeprom_read_byte(&_CV.address);
  ModuleAddress = MyModuleAddress;
  ArtNr = ARTNR;
  return LNCV_LACK_OK;
}

