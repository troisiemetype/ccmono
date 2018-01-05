// This is for testing Max7219 on the contact consequence board.

#include "TCA9534.h"

TCA9534 switches = TCA9534();

void setup(){
	switches.begin();
	switches.setDirection(0xFF);
	Serial.begin(115200);

	while(!Serial);
}

void loop(){
	switches.updateInput();

	Serial.print(switches.getPin(0));
	Serial.print('\t');
	Serial.print(switches.getPin(1));
	Serial.print('\t');
	Serial.print(switches.getPin(2));
	Serial.print('\t');
	Serial.print(switches.getPin(3));
	Serial.print('\t');
	Serial.print(switches.getPin(4));
	Serial.print('\t');
	Serial.print(switches.getPin(5));
	Serial.print('\t');
	Serial.print(switches.getPin(6));
	Serial.print('\t');
	Serial.println(switches.getPin(7));

}