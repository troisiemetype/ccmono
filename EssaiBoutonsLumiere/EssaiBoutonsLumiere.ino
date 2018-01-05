// This is for testing Max7219 on the contact consequence board.

#include "MAX7221.h"
#include "MAX7221Bargraph.h"

#include "TCA9534.h"
#include "PushButton.h"
#include "Encoder.h"


MAX7221 light;
MAX7221Bargraph lBargraph;
MAX7221Bargraph lStatus;

uint8_t intensity = 0x0;

TCA9534 ioPorts;

Encoder wheel;

PushButton bTouch;
PushButton bRelease;
PushButton bVolume;
PushButton bFadein;
PushButton bFadeout;
PushButton bMode;

int8_t barValue = 0;

void setup(){
	light.begin(11);
	light.setScanLimit(5);
	light.setIntensity(8);
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
}

void loop(){
	ioPorts.updateInput();

	if(wheel.update(ioPorts.getPin(0), ioPorts.getPin(1))){
		barValue += wheel.getStep();
		if(barValue < 0) barValue = 0;
		if(barValue > 31) barValue = 31;
		lBargraph.setValue(barValue, MAX7221Bargraph::NORMAL);
	}

	if(bTouch.update(ioPorts.getPin(2))){
		if(bTouch.justPressed()) lStatus.setLed(32);
		if(bTouch.justReleased()) lStatus.clrLed(32);
	}

	if(bRelease.update(ioPorts.getPin(3))){
		if(bRelease.justPressed()) lStatus.setLed(33);
		if(bRelease.justReleased()) lStatus.clrLed(33);
	}

	if(bVolume.update(ioPorts.getPin(4))){
		if(bVolume.justPressed()) lStatus.setLed(34);
		if(bVolume.justReleased()) lStatus.clrLed(34);
	}

	if(bFadein.update(ioPorts.getPin(5))){
		if(bFadein.justPressed()) lStatus.setLed(35);
		if(bFadein.justReleased()) lStatus.clrLed(35);
	}

	if(bFadeout.update(ioPorts.getPin(6))){
		if(bFadeout.justPressed()) lStatus.setLed(36);
		if(bFadeout.justReleased()) lStatus.clrLed(36);
	}

	if(bMode.update(ioPorts.getPin(7))){
		if(bMode.justPressed()) lStatus.setLed(37);
		if(bMode.justReleased()) lStatus.clrLed(37);
	}

}