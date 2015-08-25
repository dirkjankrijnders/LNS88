#include <LocoNet.h>
#include <S88.h>
#include "LNCV.h"
//#include "LNCV_data.h"

#define LOCONET_TX_PIN 7
#define LNCV_COUNT 10

// Item Number (Art.-Nr.): 50010
#define ARTNR 5001

//uint16_t moduleAddr;
//uint16_t lncv[LNCV_COUNT];

lnMsg *LnPacket;

LocoNetCVClass lnCV;
uint8_t* lncv;

boolean programmingMode;

S88_t S88;

LNCV_LNS88_t lnconfig EEMEM = {
  #include "LNCV_data.h"
};

void setup() {
  // put your setup code here, to run once: 
  SetupS88Hardware();
  LocoNet.init();

  lncv = (uint8_t*)&lnconfig;
  StartS88Read(&S88, FULL);
  Serial.begin(56700);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  if (IsReady(&S88)) {
    Serial.println("Ready");
    HandleS88(&S88);
    StartS88Read(&S88, FULL);
  }
  LnPacket = LocoNet.receive();
  if (LnPacket) {
    uint8_t packetConsumed(LocoNet.processSwitchSensorMessage(LnPacket));
    if (packetConsumed == 0) {
      Serial.print("Loop ");
      Serial.print((int)LnPacket);
      dumpPacket(LnPacket->ub);
      packetConsumed = lnCV.processLNCVMessage(LnPacket);
      Serial.print("End Loop\n");
    }
  }
}

void HandleS88(S88_t* S88) {
  uint8_t module = 0;
  //uint8_t half = 0;
  uint8_t current_buffer = S88->Config.activeData;
  uint8_t other_buffer = (S88->Config.activeData == 0) ? 1 : 0;
  for (module; module++; module < S88->State.maxModules) {
    if (S88->Config.data[other_buffer][module] != S88->Config.data[current_buffer][module]) {
      LocoNet.reportSensor(lnconfig.addr + lncv[1+(module*2)], S88->Config.data[current_buffer][module]);
      Serial.print("LN Sensor addr: ");
      Serial.println(lnconfig.addr + lncv[1+(module*2)]);
      Serial.print(" Old value: " );
      Serial.println(S88->Config.data[other_buffer][module]);
      Serial.print(" New value: " );
      Serial.println(S88->Config.data[current_buffer][module]);
    }
  }
}
void commitLNCVUpdate() {
  Serial.print("Module Address is now: ");
  Serial.print(lnconfig.addr);
  Serial.print("\n");
}

void dumpPacket(UhlenbrockMsg & ub) {
  Serial.print(" PKT: ");
  Serial.print(ub.command, HEX);
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
}

  /**
   * Notifies the code on the reception of a read request.
   * Note that without application knowledge (i.e., art.-nr., module address
   * and "Programming Mode" state), it is not possible to distinguish
   * a read request from a programming start request message.
   */
int8_t notifyLNCVread(uint16_t ArtNr, uint16_t lncvAddress, uint16_t,
    uint16_t & lncvValue) {
  Serial.print("Enter notifyLNCVread(");
  Serial.print(ArtNr, HEX);
  Serial.print(", ");
  Serial.print(lncvAddress, HEX);
  Serial.print(", ");
  Serial.print(", ");
  Serial.print(lncvValue, HEX);
  Serial.print(")");
  // Step 1: Can this be addressed to me?
  // All ReadRequests contain the ARTNR. For starting programming, we do not accept the broadcast address.
  if (programmingMode) {
    if (ArtNr == ARTNR) {
      if (lncvAddress < 16) {
        if (lncvAddress == 0 ){
          lncvValue = lnconfig.addr;
        } else {
          lncvValue = *(uint8_t*)(&lnconfig + 1 + lncvAddress);
        }
//        lncvValue = lncv[lncvAddress];
        Serial.print(" LNCV Value: ");
        Serial.print(lncvValue);
        Serial.print("\n");
        return LNCV_LACK_OK;
      } else {
        // Invalid LNCV address, request a NAXK
        return LNCV_LACK_ERROR_UNSUPPORTED;
      }
    } else {
      Serial.print("ArtNr invalid.\n");
      return -1;
    }
  } else {
    Serial.print("Ignoring Request.\n");
    return -1;
  }
}

int8_t notifyLNCVprogrammingStart(uint16_t & ArtNr, uint16_t & ModuleAddress) {
  // Enter programming mode. If we already are in programming mode,
  // we simply send a response and nothing else happens.
  Serial.print("notifyLNCVProgrammingStart ");
  if (ArtNr == ARTNR) {
    Serial.print("artnrOK ");
    if (ModuleAddress == lnconfig.addr) {
      Serial.print("moduleUNI ENTERING PROGRAMMING MODE\n");
      programmingMode = true;
      return LNCV_LACK_OK;
    } else if (ModuleAddress == 0xFFFF) {
      Serial.print("moduleBC ENTERING PROGRAMMING MODE\n");
      ModuleAddress = lnconfig.addr;
      return LNCV_LACK_OK;
    }
  }
  Serial.print("Ignoring Request.\n");
  return -1;
}

  /**
   * Notifies the code on the reception of a write request
   */
int8_t notifyLNCVwrite(uint16_t ArtNr, uint16_t lncvAddress,
    uint16_t lncvValue) {
  Serial.print("notifyLNCVwrite, ");
  //  dumpPacket(ub);
  if (!programmingMode) {
    Serial.print("not in Programming Mode.\n");
    return -1;
  }

  if (ArtNr == ARTNR) {
    Serial.print("Artnr OK, ");

    if (lncvAddress < 11) {
//      offset = lncvAddrress;
      if (lncvAddress == 0)
        lnconfig.addr = lncvValue;
       else
        *(uint8_t*)(&lnconfig + 1 + lncvAddress) = (uint8_t)lncvValue;
      return LNCV_LACK_OK;
    }
    else {
      return LNCV_LACK_ERROR_UNSUPPORTED;
    }

  }
  else {
    Serial.print("Artnr Invalid.\n");
    return -1;
  }
}

  /**
   * Notifies the code on the reception of a request to end programming mode
   */
void notifyLNCVprogrammingStop(uint16_t ArtNr, uint16_t ModuleAddress) {
  Serial.print("notifyLNCVprogrammingStop ");
  if (programmingMode) {
    if (ArtNr == ARTNR && ModuleAddress == lnconfig.addr) {
      programmingMode = false;
      Serial.print("End Programing Mode.\n");
      commitLNCVUpdate();
    }
    else {
      if (ArtNr != ARTNR) {
        Serial.print("Wrong Artnr.\n");
        return;
      }
      if (ModuleAddress != lnconfig.addr) {
        Serial.print("Wrong Module Address.\n");
        return;
      }
    }
  }
  else {
    Serial.print("Ignoring Request.\n");
  }
}

