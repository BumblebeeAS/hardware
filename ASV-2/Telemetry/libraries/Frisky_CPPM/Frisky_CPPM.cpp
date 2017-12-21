#include <Arduino.h>
#include "Frisky_CPPM.h"

Frisky::Frisky(int cppm_pin)
{
	_cppm_pin = cppm_pin;
	prev = 0xFFFF;  // to always discard first frame
	delta = 0;
	sync = false;
	ch = 0;
}

void Frisky::init()
{
	pinMode(_cppm_pin,INPUT);
	attachInterrupt(digitalPinToInterrupt(_cppm_pin), readppm, RISING); 
}

void Frisky::readppm() {
	if ( micros() - prev < 0 ) {  //if overflow, discard this 18ms frame
		prev = micros();
		sync = false;
		return;
	}

	delta = micros() - prev; 
	prev = micros();

	if (sync) {
		switch (_ch) {
		case 0:
			ppm[0] = delta;
			_ch++;
			break;
		case 1:
			ppm[1] = delta;
			_ch++;
			break;
		case 2:
			ppm[2] = delta;
			_ch++;
			break;
		case 3:
			ppm[3] = delta;
			_ch++;
			break;
		case 4:
			ppm[4] = delta;
			_ch++;
			break;
		case 5:
			ppm[5] = delta;
			_ch = 0;
			sync = false;
			break;
		default:
			sync = false;
			break;
		}
	} else {
		if (delta > 3000) {
			sync = true;
		} else {
			sync = false;
		}
	}
}

uint32_t Frisky::get_ch(int ch)
{
	return ppm[ch];
}