// This is for testing sound playing on CC board.

// Include for watchdog timer, to enable soft reset.
#include <avr/wdt.h>

#include "HardwareSettings.h"

#include "TCA9534.h"
#include "PushButton.h"
#include "Encoder.h"

#include "MAX7221.h"
#include "MAX7221Bargraph.h"

#include "CapacitiveADC.h"

#include <SD.h>
#include "Adafruit_VS1053.h"
#include "MAX9723.h"


int8_t barValue = 0;
uint8_t intensity = 0x0;

// Port expander
TCA9534 ioPorts;

// Rotary encoder
Encoder wheel;

// Buttons
const uint8_t numButtons = 6;
PushButton buttons[numButtons];

MAX7221 light;
MAX7221Bargraph bargraph;

// Capacitive channels
const uint8_t numSense = 4;
CapacitiveADC sense[numSense];
uint8_t touchState = 0;

// SD declaration is made automaticaly by the Arduino library

// VS1053 declaration
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(VS_RST, VS_CS, VS_DCS, DREQ, SD_CS);

// fadein/out chip
Max9723 audioOut;

const uint8_t numTrack = 4;
uint16_t fadein[numTrack] = {50, 100, 500, 4000};
uint16_t fadeout[numTrack] = {4000, 500, 100, 50};
uint16_t fadeStep = 0;
uint32_t fadeCounter = 0;
uint8_t fadeVolume = 1;
const uint8_t minFadeVolume = 1;
const uint8_t maxFadeVolume = 31;

int8_t currentTrack = -1;


uint8_t volume = 50;

enum state_t{
	INIT = 0,
	IDLE,
	FADE_IN,
	PLAYING,
	FADE_OUT,
	SETTING,
	RESET,
} globalState = INIT;

// TODO: manage touch = play & release = stop / touch = play & touch again = stop
enum mode_t{
	ONE_TOUCH = 0,
	ONE_PROX,
	TWO_TOUCH,
	TWO_PROX,
	FADE,
} mode = ONE_TOUCH;

enum priority_t{
	FIRST = 0,
	LAST,
	ALL,
} priority = FIRST;

/*
enum fade_t{
	FADE_IDLE = 0,
	FADE_IN,
	FADE_OUT,
} fadeState = FADE_IDLE;
*/
void setup(){
	globalState = INIT;

	Serial.begin(115200);
//	while(!Serial);

	// Set the MAX9723 chip, set volume to minimum (0 disconnect the chip and gives a noise).
	audioOut.begin();
	audioOut.setGain(0);
	audioOut.setBass(0);
	audioOut.setVolume(minFadeVolume);

	// Init port expander, rotary encoder and buttons.
	ioPorts.begin();
	ioPorts.setDirection(0xff);

	wheel.begin(Encoder::DOUBLE_STEP);

	for(uint8_t i = 0; i < 6; ++i){
		buttons[i].begin(PULLUP);
	}

	// Init led multiplexer MAX7219, set limit, set Intensity, clear display.
	light.begin(MAX_CS);
	light.setScanLimit(5);
	light.setIntensity(0);
	for(uint8_t i = 0; i < 8; ++i){
		light.setDigit(i, 0);
	}

	// Init bargraph and blink it to show startup.
	bargraph.begin(&light);
	bargraph.init(0, 31);

	for(uint8_t i = 0; i < 3; ++i){
		bargraph.setValue(32);
		delay(100);
		bargraph.clrAll();
		delay(100);	
	}

	// Init sense touches
	sense[0].init(SENSE1, SENSE2);
	sense[1].init(SENSE2, SENSE1);
	sense[2].init(SENSE3, SENSE1);
	sense[3].init(SENSE4, SENSE1);

	// and set their default parameters
	// TODO: we have to read this parameter from SD card on startup.
	for(uint8_t i = 0; i < numSense; ++i){
		sense[i].setTouchThreshold(150);
		sense[i].setTouchReleaseThreshold(90);
		sense[i].setProxThreshold(14);
		sense[i].setProxReleaseThreshold(8);
		sense[i].setChargeDelay(5);
		sense[i].setDebounce(5);

		sense[i].tuneBaseline();
	}

	// Init the music player
	if (! player.begin()) {
		// If it can't init, we display fading center bargraph to show it.
		for(uint8_t i = 0; i < 33; ++i){
			bargraph.setValue(32 - i, MAX7221Bargraph::CENTER);
			delay(20);
		}
		bargraph.clrAll();

		// And we enable a long push on mode button to reset the board.
		while (1){
			ioPorts.updateInput();
			if(buttons[5].update(ioPorts.getPin(7))){
				if(buttons[5].isLongPressed()){
					reset();
				}
			}
		}
	}

	// If the player has succesfully start, we display a growing bargraph, left justified.
	for(uint8_t i = 0; i < 16; ++i){
		bargraph.setValue(i);
		delay(20);
	}

	// We set the volume and the interrupts for the music player
	player.setVolume(volume, volume);
	player.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

	// And set back volume to max.
//	audioOut.setVolume(31);

	// Last, we have to init the SD card.
	if (!SD.begin(SD_CS)) {

	}

	// If SD init is successfull, we finish the bargrah growth.
	for(uint8_t i = 16; i < 33; ++i){
		bargraph.setValue(i);
		delay(20);
	}
	delay(250);
	bargraph.clrAll();

	globalState = IDLE;

}

void loop(){

//	Serial.println(globalState);
	light.setDigit(4, 0);
	light.setDigit(4, (1 << globalState - 1));

	switch(globalState){
		case IDLE:
			loopIdle();
			break;
		case FADE_IN:
			loopPlaying();
			break;
		case PLAYING:
			loopPlaying();
			break;
		case FADE_OUT:
			loopPlaying();
			break;
		case SETTING:
			loopSetting();
			break;
		case RESET:
			break;
		default:
			break;
	}

}

void loopIdle(){
	ioPorts.updateInput();
	testReset();

	if(buttons[4].update(ioPorts.getPin(6))){
		if(buttons[4].isLongPressed()){
			sense[0].tuneThreshold(3000);
			SettingsLocal_t settings = sense[0].getLocalSettings();
			for(uint8_t i = 0; i < numSense; ++i){
				sense[i].applyLocalSettings(settings);
			}
		}
	}

	uint8_t update = updateReads();
	if(update && touchState){
		for(uint8_t i = 0; i < numSense; ++i){
			if(!(update & _BV(i))) continue;

			if(touchState & (0b01 << (2 * i))){
				switch(mode){
					case ONE_PROX:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case TWO_PROX:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case FADE:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					default:
						break;
				}
			} else if( touchState & (0b10 << (2 * i))){
				switch(mode){
					case ONE_TOUCH:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case TWO_TOUCH:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case FADE:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					default:
						break;
				}
			}
		}
	}

}

void loopPlaying(){
	ioPorts.updateInput();
	testReset();

	if((globalState == FADE_IN) || (globalState == FADE_OUT)) fade();
	if(globalState == IDLE) return;

	uint8_t update = updateReads();

	if(update){
		for(uint8_t i = 0; i < numSense; ++i){
			if(!(update & _BV(i))) continue;

			if(touchState & (0b01 << (2 * i))){
				switch(mode){
					case ONE_PROX:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case TWO_PROX:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case FADE:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					default:
						break;
				}
			} else if(touchState & (0b10 << (2 * i))){
				switch(mode){
					case ONE_TOUCH:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case TWO_TOUCH:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					case FADE:
						globalState = FADE_IN;
						startPlay(i);
						startFade(i);
						break;
					default:
						break;
				}
			} else if(!(touchState & (0b00 << (2 * i)))){
			Serial.println("FADE OUT");
				switch(mode){
					case ONE_TOUCH:
						globalState = FADE_OUT;
						startFade(i);
						break;
					case TWO_TOUCH:
						globalState = FADE_OUT;
						startFade(i);
						break;
					case FADE:
						globalState = FADE_OUT;
						startFade(i);
						break;
					default:
						break;
				}

			}
		}
	}

}

void loopSetting(){

}

uint8_t updateReads(){
	uint8_t update = false;
	uint8_t previousState = touchState;

	for(uint8_t i = 0; i < numSense; i++){
		uint8_t mask = 0b11 << (2 * i);
		uint8_t state = 0;

		sense[i].update();

		if(sense[i].isJustProx()){
			state = 0b01 << (2 * i);
//			Serial.println("prox");
		} else if(sense[i].isJustTouched()){
			state = 0b10 << (2 * i);
//			Serial.println("touch");
		} else if(sense[i].isJustReleased()){
			state = 0b00 << (2 * i);
//			Serial.println("air");
		} else {
			continue;
		}
		
		switch(priority){
			case FIRST:
				if(!(touchState & ~mask)){
					touchState &= ~mask;
					touchState |= state;
					update &= ~_BV(i);
					update |= _BV(i);
				}
				break;
			case LAST:
				touchState = state;
				update = _BV(i);		
				break;
			case ALL:
				touchState &= ~mask;
				touchState |= state;
				update &= ~_BV(i);
				update |= _BV(i);
				break;
			default:
				break;
		}
	}

//	if(touchState != previousState) update = true;

	return update;
}

uint8_t startPlay(uint8_t track){
	Serial.println("FADE IN");
	if(player.playingMusic){
		if(currentTrack != track){
		player.stopPlaying();			
		} else {
			return 0;
		}
	}

	char *name = "track000.mp3";
	name[7] = track + '1';
	player.startPlayingFile(name);
	currentTrack = track;
	return 0;
}

uint8_t stopPlay(){
	player.stopPlaying();
	currentTrack = -1;
	globalState = IDLE;
	Serial.println("IDLE");
	return 0;
}

uint8_t fade(){
//	Serial.println(fadeCounter + fadeStep);
//	Serial.println(millis());
	if(((fadeCounter + fadeStep) > millis())) return 0;
	fadeCounter = millis();
	switch(globalState){
		case FADE_IN:
			if(++fadeVolume >= maxFadeVolume){
				fadeVolume = maxFadeVolume;
				globalState = PLAYING;
				Serial.println("PLAYING");
			}
			break;
		case FADE_OUT:
			if(--fadeVolume <= minFadeVolume){
				fadeVolume = minFadeVolume;
				stopPlay();
			}
//			Serial.println(fadeVolume);
			break;
	}
	audioOut.setVolume(fadeVolume);
//	Serial.print("fade: ");
//	Serial.println(fadeVolume);
	return 0;
}

uint8_t startFade(uint8_t track){
	if(globalState == FADE_IN){
		fadeStep = fadein[track] / 32;
	} else if(globalState == FADE_OUT){
		fadeStep = fadeout[track] / 32;		
	}
//	Serial.println(fadeStep);
	fadeCounter = millis();
//	Serial.print("step: ");
//	Serial.println(fadeStep);
//	Serial.print("counter: ");
//	Serial.println(fadeCounter);
	return 0;
}

void testReset(){
	if(buttons[5].update(ioPorts.getPin(7))){
		if(buttons[5].isLongPressed()){
			reset();
		}
	}

}
void reset(){
//	asm volatile("jmp	0");
	wdt_enable(WDTO_60MS);
	while(1);

}