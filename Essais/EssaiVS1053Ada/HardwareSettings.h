/*
 * This file is part of Contact&Consequence.
 * Copyright (C) 2017  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is the setting file for the Contact & Consequence capacitive music player.
 * It groups all the pins definitions in one file, as well as other settings.
 */

#ifndef HARDWARE_SETTINGS_H
#define HARDWARE_SETTINGS_H

// Sense pins
#define SENSE1 			A4			// PF1 / D22
#define SENSE2 			A3			// PF4 / D21
#define SENSE3 			A2			// PF5 / D20
#define SENSE4 			A1			// PF6 / D19

// IO pins
#define IO1 			SENSE3
#define IO2 			SENSE4

// Analog pin
#define BATT_LEVEL		A5			// PF0 / D23

// Communication
#define SDA				2			// PD1
#define SCL				3			// PD0

#define MISO			14			// PB3
#define MOSI			16			// PB2
#define CLK				15			// PB1

#define SD_CS 			17			// PB0
#define IO_INT			8			// PB4
#define MAX_CS			11 			// PB7
#define VS_CS			4			// PD4
#define VS_DCS			30			// PD5
#define VS_RST			31			// PE2 -> custom variant file to add pin 31 for PE2
#define DREQ			7			// PE6
#define VS_MIDI_MODE	18			// PF7





#endif