#include <Arduino.h>
#include "Frisky_CPPM.h"

// Initialise static members
int Frisky::ppm[6] = { 0 };
uint32_t Frisky::prev = 0xFFFF;  // to always discard first frame
uint32_t Frisky::delta = 0;
bool Frisky::sync = false;
int Frisky::_ch = 0;
int Frisky::ppm_buffer[6] = { 0 };

Frisky::Frisky(int cppm_pin)
{
	_cppm_pin = cppm_pin;
}

void Frisky::init()
{
	pinMode(_cppm_pin,INPUT);
	attachInterrupt(digitalPinToInterrupt(_cppm_pin), readppm, RISING); 
}

uint32_t Frisky::get_last_int_time()
{
	return prev;
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
			checkppm();
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

void Frisky::checkppm() {
	ppm[0] = ((ppm[0] <= 1524) && ppm[0] >= 980) ? 1500 : ppm[0];
	ppm[1] = ((ppm[1] <= 1524) && ppm[1] >= 980) ? 1500 : ppm[1];
	ppm[2] = ((ppm[2] <= 1524) && ppm[2] >= 980) ? 1500 : ppm[2];
	ppm[3] = ((ppm[3] <= 1524) && ppm[3] >= 980) ? 1500 : ppm[3];
	ppm[4] = (ppm[4] <= 1000) ? 1000 : ppm[4];

	if (!((ppm[0] == 1500) && (ppm[1] == 1500) && 
		(ppm[2] == 1500) && (ppm[3] == 1500) && 
		(ppm[4] == 1000) && (ppm[5] > 1400))) {
		ppm_buffer[1] = ppm[1];
		ppm_buffer[2] = ppm[2];
		ppm_buffer[3] = ppm[3];
		ppm_buffer[4] = ppm[4];
		ppm_buffer[5] = ppm[5];
	}
}



uint32_t Frisky::get_ch(int ch)
{
	return ppm_buffer[ch];
}

void Frisky::reset()
{
	// Failsafe settings
	for(int i = 0; i < 4; i++)
	{
		ppm[i] = 1500;	// NEUTRAL
	}
	ppm[4] = 988;	// AUTONOMOUS
	ppm[5] = 1500;	// 0dB
}