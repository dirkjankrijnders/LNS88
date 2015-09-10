#include <LocoNet.h>

#include <S88.h>

lnMsg        *LnPacket;
S88_t S88;

void setup() {
  // put your setup code here, to run once: 
  DDRB |= (1 << PB5);
  PORTB ^= (1 << PB5);
  SetupS88Hardware(&S88);
  PORTB ^= (1 << PB5);
//  LocoNet.init();
  PORTB ^= (1 << PB5);
  setNoModules(&S88, 0, 3);
  //S88.State.maxModules = 3;
  StartS88Read(&S88, FULL);
  PORTB ^= (1 << PB5);
}

void loop() {
  // put your main code here, to run repeatedly:
  //PORTB ^= (1 << 5);

  if (IsReady(&S88)) {
    StartS88Read(&S88, FULL);
  }
}

int main() {
  DDRB |= (1 << 5);
  //PORTB ^= (1 << 5);
	setup();
	while (1) {
//  PORTB ^= (1 << 5);
		loop();
	}
};

