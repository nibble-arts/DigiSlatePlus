/*
 DigiSlatePlus

 SMPTE/EBU Timecode class
 */

/*
This Source Code Form is subject to the terms of the
Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed
with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

Copyright (c) 2024 Thomas Winkler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "tc.h"


// =============================================================
// init timecode
void TC::begin(void) {

	// reset counters
	_start = true;
	_level = false;

	_enable = false;

	_length = 0;

	_bit_counter = 0;
	_byte_counter = 0;

	// set standards
	_tc.dropframe = false;
	_tc.colorframe = false;

	// init binary data
	for (_i = 0; _i < 8; _i++) {
		_binary[_i] = 0b10000000;
	}

	// set sync word
	// 0011 1111 1111 1101
	_binary[8] = 0b11111100;
	_binary[9] = 0b10111111;


	// clear userbits
	for (_i = 0; _i < 8; _i++) {
		_ub.bit[_i] = 0x00;
	}

	update_binary();
}


// =============================================================
// enable/disabel timecode output
void TC::enable(bool ea) {
	_enable = ea;
}

bool TC::enable(void) {
	return _enable;
}


// =============================================================
// timecode
void TC::set(TIMECODE tc) {
	set(tc.h, tc.m, tc.s, tc.f, tc.fps);
}

// set timecode with framerate
void TC::set(uint8_t h, uint8_t m, uint8_t s, uint8_t f, uint8_t fr) {
	set (h, m, s, f);
	fps(fr);
}

// set timecode
void TC::set(uint8_t h, uint8_t m, uint8_t s, uint8_t f) {
	_tc.h = h;
	_tc.m = m;
	_tc.s = s;
	_tc.f = f;

	_tc.changed = true;
}

// get timecode struct
TIMECODE TC::get(void) {
	return _tc;
}


void TC::update_binary(void) {

	// SMPTE/EBU timecode structure (https://www.wikiaudio.org/smpte-time-code/)

	// ** byte 0
	//  0-3	frame units
	//  4-7	user bits 1
	_binary[0] = ((_tc.f % 10) & 0b1111) | ((_ub.bit[0] & 0xF) << 4);
	// _binary[0] = (((_tc.f % 10) & 0b1111) << 4) | (_ub.bit[0] & 0xF);
	
	// ** byte 1
	//  8-9	frame tens
	//  10		dropframe bit 	1=drop frame (frame 0 and 1 omitted from first second of each minute, but included when minutes divides by ten; approximates 29.97 frame/s)
	//  11 	colorframe bit 	i.e. the time code is intentionally synchronised with a color TV field sequence.
	//  12-15	user bits 2
	_binary[1] = ((_tc.f / 10) & 0b0011) | (_tc.dropframe << 2) | (_tc.colorframe << 3) | ((_ub.bit[1] & 0xF) << 4);
	// _binary[1] = ((_tc.f / 10) & 0b0011) << 6 | (_tc.dropframe << 5) | (_tc.colorframe << 4) | (_ub.bit[1] & 0xF);

	// ** byte 2
	//  16-19	secs units
	//  20-23	user bits 3
	_binary[2] = ((_tc.s % 10) & 0b1111) | ((_ub.bit[2] & 0xF) << 4);
	// _binary[2] = (((_tc.s % 10) & 0b1111) << 4) | (_ub.bit[2] & 0xF);

	// ** byte 3
	//  24-26	secs tens
	//  27		bi phase mark correction bit 
	//  28-31	user bits 4
	_binary[3] = ((_tc.s / 10) & 0b0111) | (_tc.biphase << 3) | ((_ub.bit[3] & 0xF) << 4);
	// _binary[3] = (((_tc.s / 10) & 0b0111) << 4) | (_tc.biphase << 3) | (_ub.bit[3] & 0xF);

	// ** byte 4
	//  32-35	mins units
	//  36-39	user bits 5
	_binary[4] = ((_tc.m % 10) & 0b1111) | ((_ub.bit[4] & 0xF) << 4);
	// _binary[4] = (((_tc.m % 10) & 0b1111) << 4) | (_ub.bit[4] & 0xF);

	// ** byte 5
	//  40-42	mins tens
	//  43		binary group flag bit (with bit 59, 43,59 = 00 = no format for user bits, 10 = eight bit format, 01, 11 are unassigned and reserved).
	//  44-47	user bits 6
	_binary[5] = ((_tc.m / 10) & 0b0111) | (_tc.flag0 << 3) | ((_ub.bit[5] & 0xF) << 4);
	// _binary[5] = (((_tc.m / 10) & 0b0111) << 4) | (_tc.flag0 << 3) | (_ub.bit[5] & 0xF);

	// ** byte 6
	//  48-51	hours units
	//  52-55	user bits 7
	_binary[6] = ((_tc.h % 10) & 0b1111) | ((_ub.bit[6] & 0xF) << 4);
	// _binary[6] = (((_tc.h % 10) & 0b1111) << 4) | (_ub.bit[6] & 0xF);

	// ** byte 7
	//  56-57	hour tens
	//  58		unused, reserved, should transmit zero and ignore on receive for compatibility
	//  59		binary group flag bit (see bit 43 for encoding)
	//  60-63	user bits 8
	_binary[7] = ((_tc.h / 10) &0b0011) | 0b000000100 | (_tc.flag1 << 3) | ((_ub.bit[7] & 0xF) << 4);
	// _binary[7] = (((_tc.h / 10) &0b0011) << 4) | 0b000000100 | (_tc.flag1 << 3) | (_ub.bit[7] & 0xF);

	// ** byte 8 - 9
	//  64-79	sync word (should be 0011 1111 1111 1101)
	// set in timecode init
}



// =============================================================
// framerate
// set framerate
void TC::fps(uint8_t fr) {
	_tc.fps = fr;
}

// get framerate
uint8_t TC::fps(void) {
	return _tc.fps;
}


// =============================================================
// user bits
// set userbits from char array
void TC::ubits(USERBITS* ub) {
	for (uint8_t i = 0; i < 8; i++) {
		ubit(i, ub->bit[i]);
	}
}

// set userbits by 4 bit values
void TC::ubits(uint8_t ub0, uint8_t ub1, uint8_t ub2, uint8_t ub3, uint8_t ub4, uint8_t ub5, uint8_t ub6, uint8_t ub7) {
	ubit(0, ub0 & 0xF);
	ubit(1, ub1 & 0xF);
	ubit(2, ub2 & 0xF);
	ubit(3, ub3 & 0xF);
	ubit(4, ub4 & 0xF);
	ubit(5, ub5 & 0xF);
	ubit(6, ub6 & 0xF);
	ubit(7, ub7 & 0xF);
}

// set userbit by index
void TC::ubit(uint8_t nr, uint8_t ub) {
	if (nr < 8) {
		_ub.bit[nr] = ub;
	}
}

// get userbit struct
USERBITS TC::ubits(void) {
	return _ub;
}

// get userbit by index
uint8_t TC::ubit(uint8_t nr) {
	if (nr < 8) {
		return _ub.bit[nr];
	}
	return false;
}


// =============================================================
// inc TC
// do not increment when tick is false
// tick is set to true, if the rtc second interrupt occured
long TC::inc(bool tick) {


	// set length to 0
	// is only returned when frame starts
	// _length = 0;

	// =======================================
	// send_bit(_bit_count);
	bool bit = _binary[_byte_counter] & (1 << _bit_counter);
	// bool bit = _binary[_byte_counter] & (1 << (7 - _bit_counter));


	// is 1 and mid bit => change polarity
	if (bit == true && !_start) {
		_level = !_level;
	}


	// is bit start => change porality and select next bit
	if (_start) {
		_level = !_level;
		_bit_counter++;
	}

	_start = !_start;


	// =======================================
	// write to signal output
	// if timecode is enabled
	// else set to low
	if (_enable) {
		digitalWrite(SIGNAL_OUTPUT, _level);
	}
	else {
		digitalWrite(SIGNAL_OUTPUT, LOW);
	}
	// =======================================


	// =======================================
	// bit overflow => next byte
	if (_bit_counter >= GEN_MAX_BITS) {

		_bit_counter = 0;
		_byte_counter++;


		// last byte => start from begin
		// =======================================
		if (_byte_counter >= GET_MAX_BYTES) {


			// =======================================
			// FRAME BEGIN
			// =======================================

			_byte_counter = 0;


// Debug => write frame sync to flash led
// digitalWrite(FLASH_LED, HIGH);
// // delay(1);
// digitalWrite(FLASH_LED, LOW);

			_tc.f++;

			// frames overflow
			// second is over
			if (_tc.f >= _tc.fps) {

				_tc.f = 0;
				_tc.s++;

				// calculate frame time
				_time = micros();
				_length = _time - _old_time;
				_old_time = _time;

			}

			if (_tc.s > 59) {
				_tc.s = 0;
				_tc.m++;
			}

			if (_tc.m > 59) {
				_tc.m = 0;
				_tc.h++;
			}

			if (_tc.h > 23) {
				_tc.f = 0;
				_tc.s = 0;
				_tc.m = 0;
				_tc.h = 0;
			}

			_tc.changed = true;
			update_binary();
		}
	}

	return _length;
}


// return tc status
bool TC::changed(void) {
	return _tc.changed;
}

void TC::unchange(void) {
	_tc.changed = false;
}