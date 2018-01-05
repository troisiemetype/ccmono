// This is for testing sound playing on CC board.

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

// SD declaration is made automaticaly by the Arduino library

// VS1053 declaration
Adafruit_VS1053_FilePlayer player = Adafruit_VS1053_FilePlayer(VS_RST, VS_CS, VS_DCS, DREQ, SD_CS);

// fadein/out chip
Max9723 audioOut;


int16_t proxSensivity = 10;
int16_t touchSensivity = 100;

int16_t baseline = 800;
int16_t maxDelta = 25;

int8_t charge = 3;

uint16_t tempo = 150;
uint32_t nextTempo = millis() + tempo;

uint8_t volume = 50;

void setup(){

	Serial.begin(115200);
	while(!Serial);

	pinMode(VS_MIDI_MODE, OUTPUT);
	digitalWrite(VS_MIDI_MODE, LOW);

	ioPorts.begin();
	ioPorts.setDirection(0xff);

	wheel.begin(Encoder::DOUBLE_STEP);

	for(uint8_t i = 0; i < 6; ++i){
		buttons[i].begin(PULLUP);
	}

	light.begin(MAX_CS);
	light.setScanLimit(4);
	light.setIntensity(0);
	for(uint8_t i = 0; i < 8; ++i){
		light.setDigit(i, 0);
	}

	bargraph.begin(&light);
	bargraph.init(0, 31);

	bargraph.setValue(10);

	sense[0].init(SENSE1, SENSE2);
	sense[1].init(SENSE2, SENSE1);
	sense[2].init(SENSE3, SENSE1);
	sense[3].init(SENSE4, SENSE1);

	for(uint8_t i = 0; i < numSense; ++i){
		sense[i].setTouchThreshold(150);
		sense[i].setTouchReleaseThreshold(90);
		sense[i].setProxThreshold(proxSensivity);
		sense[i].setProxReleaseThreshold(proxSensivity / 2);
		sense[i].setChargeDelay(charge);
		sense[i].setDebounce(5);

		sense[i].tuneBaseline();
	}

	audioOut.begin();
//	audioOut.setGain(1);
	audioOut.setVolume(1);

	if (! player.begin()) { // initialise the music player
//	Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
		for(uint8_t i = 0; i < 32; ++i){
			bargraph.setValue(32 - i, MAX7221Bargraph::CENTER);
			delay(20);
		}
		bargraph.clrAll();
		while (1){
			ioPorts.updateInput();
			if(buttons[5].update(ioPorts.getPin(7))){
				if(buttons[5].isLongPressed()){
					reset();
				}
			}

		}
	}

	for(uint8_t i = 0; i < 32; ++i){
		bargraph.setValue(i);
		delay(20);
	}
	bargraph.clrAll();
//	Serial.println(F("VS1053 found"));

	player.setVolume(volume, volume);
	player.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

	audioOut.setVolume(31);

	bargraph.setValue(20);

	if (!SD.begin(SD_CS)) {
//		Serial.println(F("SD failed, or not present"));
//		while (1);  // don't do anything more
	}
/*
	player.sineTest(0x44, 300);
	delay(200);
	player.sineTest(0x44, 300);
	delay(200);
*/
	// list files
//	printDirectory(SD.open("/"), 0);
/*
	Serial.println(F("Playing track 001"));
	player.playFullFile("track001.mp3");
	Serial.println(F("Playing finished"));
*/
}

void loop(){

	ioPorts.updateInput();


	if(buttons[4].update(ioPorts.getPin(6))){
		if(buttons[4].isLongPressed()){
			sense[0].tuneThreshold(3000);
			SettingsLocal_t settings = sense[0].getLocalSettings();
			proxSensivity = settings.proxThreshold;
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

	if(wheel.update(ioPorts.getPin(0), ioPorts.getPin(1))){
		volume -= wheel.getStep();
		player.setVolume(volume, volume);
		bargraph.setValue(31 - volume);
	}

	if(buttons[1].update(ioPorts.getPin(3))){
		if(buttons[1].justPressed()) audioOut.setGain(!audioOut.getGain());
	}
/*
	if(wheel.update(ioPorts.getPin(0), ioPorts.getPin(1))){
		audioOut.setVolume(audioOut.getVolume() + wheel.getStep());
	}
*/


	for(uint8_t i = 0; i < numSense; i++){
		sense[i].update();
		if(sense[i].isJustTouched()){
//			if(!player.playingMusic){
				char *track = "track000.mp3";
				track[7] = i + '1';
				player.startPlayingFile(track);
//			} else {
//				player.stopPlaying();
//			}
		} else if(sense[i].isJustTouchedReleased()){
			player.stopPlaying();
		}
	}


//	sense2.update();

}

void reset(){
	asm volatile(
		"jmp	0"		"\n\t"
	);
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}