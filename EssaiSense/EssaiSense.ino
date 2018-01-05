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
MAX7221Bargraph lBargraph;
MAX7221Bargraph lStatus;

int8_t barValue = 0;
uint8_t intensity = 0x0;

// Port expander
TCA9534 ioPorts;

// Rotary encoder
Encoder wheel;

// Buttons
PushButton bTouch;
PushButton bRelease;
PushButton bVolume;
PushButton bFadein;
PushButton bFadeout;
PushButton bMode;

// Capacitive channels
CapacitiveADC sense1;
CapacitiveADC sense2;
//CapacitiveADC sense3;
//CapacitiveADC sense4;

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

	lBargraph.begin(&light);
	lBargraph.init(0, 31);

	lStatus.begin(&light);
	lStatus.init(32, 37);

	ioPorts.begin();
	ioPorts.setDirection(0xff);

	wheel.begin(Encoder::DOUBLE_STEP);

	bTouch.begin(PULLUP);
	bRelease.begin(PULLUP);
	bVolume.begin(PULLUP);
	bFadein.begin(PULLUP);
	bFadeout.begin(PULLUP);
	bMode.begin(PULLUP);

	sense1.init(SENSE1, SENSE2);
	sense1.setTouchThreshold(150);
	sense1.setTouchReleaseThreshold(90);
	sense1.setProxThreshold(proxSensivity);
	sense1.setProxReleaseThreshold(proxSensivity / 2);
	sense1.setChargeDelay(charge);

	sense1.tuneBaseline();

	sense2.init(SENSE2, SENSE1);

	sense2.tuneBaseline();


	Serial.begin(115200);
}

void loop(){

	ioPorts.updateInput();
/*
	if(wheel.update(ioPorts.getPin(0), ioPorts.getPin(1))){
		charge += wheel.getStep();
		if(charge < 0) charge = 0;
		if(charge > 32) charge = 32;
		sense1.setChargeDelay(charge);
		lStatus.setValue(charge);
	}
*/
	if(wheel.update(ioPorts.getPin(0), ioPorts.getPin(1))){
		proxSensivity += wheel.getStep();
		if(proxSensivity < 0) proxSensivity = 0;
		if(proxSensivity > 800) proxSensivity = 800;
		sense1.setProxThreshold(proxSensivity);
		sense2.setProxReleaseThreshold(proxSensivity / 2);
		lBargraph.setValue(proxSensivity / 2);
	}

	if(bMode.update(ioPorts.getPin(7))){
		if(bMode.isLongPressed()){
			lBargraph.clrAll();
			lStatus.clrAll();
			sense1.tuneThreshold(5000);
			baseline = sense1.getBaseline();
			maxDelta = sense1.getMaxDelta();
			maxDelta /= 32;
			SettingsLocal_t settings = sense1.getLocalSettings();
			proxSensivity = settings.proxThreshold;
		}
	}

	bFadeout.update(ioPorts.getPin(6));
	if(bFadeout.isPressed()){
//		lBargraph.setValue(analogRead(BATT_LEVEL) / 16);
		delay(1000);
	}
	
//	uint32_t time = millis();

	int16_t senseValue = sense1.update();

//	Serial.println(millis() - time);

//	Serial.println(senseValue);
	if(senseValue < 0) senseValue = 0;
	lBargraph.setValue((senseValue) / maxDelta);
//	Serial.print(charge);
//	Serial.print(":\t");
//	Serial.println(senseValue);

	lStatus.setLed(33, sense1.isTouched());
	lStatus.setLed(32, sense1.isProx());

//	sense2.update();

}