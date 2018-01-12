// This is for testing capacitive sensing on CC board.

#include "HardwareSettings.h"
#include "MAX7221.h"
#include "MAX7221Bargraph.h"

#include "TCA9534.h"
#include "PushButton.h"
#include "Encoder.h"

#include "CapacitiveADC.h"

// Led driver
MAX7221 light;
MAX7221Bargraph bargraph;

int8_t barValue = 0;
uint8_t intensity = 0x0;

// Port expander
TCA9534 ioPorts;

// Rotary encoder
Encoder wheel;

// Buttons
const uint8_t numButtons = 6;
PushButton buttons[numButtons];

// Capacitive channels
const uint8_t numSense = 1;
CapacitiveADC sense[numSense];

int16_t proxSensivity = 10;
int16_t touchSensivity = 100;

int16_t baseline = 800;
int16_t maxDelta = 25;

int8_t charge = 3;

void setup(){
	light.begin(11);
	light.setScanLimit(5);
	light.setIntensity(1);
	for(uint8_t i = 0; i < 8; ++i){
		light.setDigit(i, 0);
	}

	bargraph.begin(&light);
	bargraph.init(0, 31);

	ioPorts.begin();
	ioPorts.setDirection(0xff);

	wheel.begin(Encoder::DOUBLE_STEP);

	for(uint8_t i = 0; i < 6; ++i){
		buttons[i].begin(PULLUP);
	}

	sense[0].init(SENSE1, SENSE2);
//	sense[1].init(SENSE2, SENSE1);
//	sense[2].init(SENSE3, SENSE1);
//	sense[3].init(SENSE4, SENSE1);


	for(uint8_t i = 0; i < numSense; ++i){
		sense[i].setTouchThreshold(240);
		sense[i].setTouchReleaseThreshold(160);
		sense[i].setProxThreshold(16);
		sense[i].setProxReleaseThreshold(8);
		sense[i].setChargeDelay(charge);
		sense[i].setDebounce(5);

		sense[i].tuneBaseline();
	}


	Serial.begin(115200);
//	while(!Serial);
}

void loop(){

	ioPorts.updateInput();

	if(buttons[4].update(ioPorts.getPin(6))){
		if(buttons[4].isLongPressed()){
			light.setDigit(0, 0);
			light.setDigit(1, 0);
			light.setDigit(2, 0);
			light.setDigit(3, 0);
			sense[0].tuneThreshold(3000);
			SettingsLocal_t settings = sense[0].getLocalSettings();
			for(uint8_t i = 0; i < numSense; ++i){
				sense[i].applyLocalSettings(settings);
			}
		}
	}

	if(buttons[5].update(ioPorts.getPin(7))){
		if(buttons[5].isLongPressed()){
			reset();
		}
	}
//	uint32_t length = micros();
/*
	uint32_t length = 0;
//	length = micros();
//	Serial.println(micros() - length);
	length = micros();
	sense[1].update();
	Serial.println(micros() - length);
*/
	for(uint8_t i = 0; i < numSense; i++){
		sense[i].update();
		if(sense[i].isProx()){
//			light.setDigit(i, 7);
			bargraph.setValue(sense[i].proxRatio() / 10);
		} else if(sense[i].isTouched()){
//			light.setDigit(i, 63);
			bargraph.setValue(32);
		} else {
//			light.setDigit(i, 1);
			bargraph.clrAll();
		}
	}

//	Serial.println(micros() - length);

}

void reset(){
	asm volatile(
		"jmp	0"		"\n\t"
	);
}