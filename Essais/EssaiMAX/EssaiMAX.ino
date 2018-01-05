// This is for testing Max7219 on the contact consequence board.

#include "MAX7221.h"
#include "MAX7221Bargraph.h"

MAX7221 light;

MAX7221Bargraph bargraph;

uint8_t intensity = 0x0;

uint8_t start = 0;


void setup(){
	light.begin(11);
	light.setScanLimit(5);
	light.setIntensity(0);
	for(uint8_t i = 0; i < 8; ++i){
		light.setDigit(i, 0);
	}

	bargraph.begin(&light);
	bargraph.init(0, 31);
}

void loop(){
	for(uint8_t i = 0; i < 32; ++i){
		bargraph.setLed(i);
		delay(10);
	}

	for(uint8_t i = 0; i < 32; ++i){
		bargraph.clrLed(i);
		delay(10);
	}

	for(uint8_t i = 0; i < 27; ++i){
		bargraph.setArea(i, i + 5);
		delay(10);
		bargraph.clrArea(i, i + 5);
	}

	for(uint8_t i = 0; i < 32; ++i){
		bargraph.setValue(i);
		delay(5);
	}

	for(int8_t i = 31; i >= 0; --i){
		bargraph.setValue(i);
		delay(5);
	}

}