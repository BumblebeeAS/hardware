//===========================================
//
//    Library to read from DBR-XP receiver using 8 channels
//
//===========================================

#include <Arduino.h>
#include "Frisky_CPPM_ASV3.h"

// Initialise static members
int Frisky::ppm[8] = { 0 };
uint32_t Frisky::prev = 0xFFFF;  // to always discard first frame			
uint32_t Frisky::delta = 0;
bool Frisky::sync = false;
int Frisky::_ch = 0;
int Frisky::ppm_buffer[8] = { 0};
uint8_t Frisky::counter = 0;

// Declare input pin to read cppm from 
Frisky::Frisky(int cppm_pin)
{
	_cppm_pin = cppm_pin;
}

// Initialize cppm pin as input interrupt 
void Frisky::init()
{
	pinMode(_cppm_pin,INPUT);
	// call readppm at each rising edge. 
	// cppm protocol -> each channel is sent at 20ms intervals, with the value being the width of pulse (like pwm)
	attachInterrupt(digitalPinToInterrupt(_cppm_pin), readppm, RISING); 
}

uint32_t Frisky::get_last_int_time()
{
	return prev;
}

// read ppm is called for each channel (0 - 7)
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
			_ch++;						// after reading 1 channel, the next pulse is the subsequent channel to move on to next channel
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
      _ch++;
      break;
    case 6:
      ppm[6] = delta;
      _ch++;
      break;		
    case 7:
			ppm[7] = delta;
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

inline void Frisky::checkppm() {
  // deal with variations in default values
	ppm[0] = ((ppm[0] <= 1524) && (ppm[0] >= 1476)) ? 1500 : ppm[0];     
	ppm[1] = ((ppm[1] <= 1524) && (ppm[1] >= 1476)) ? 1500 : ppm[1];
	ppm[2] = ((ppm[2] <= 1524) && (ppm[2] >= 1476)) ? 1500 : ppm[2];
	ppm[3] = ((ppm[3] <= 1524) && (ppm[3] >= 1476)) ? 1500 : ppm[3];
	ppm[4] = (ppm[4] <= 1000) ? 1000 : ppm[4];
  ppm[6] = (ppm[6] <= 1000) ? 1000 : ppm [6];
  if (ppm[7] > 1600) {
    ppm[7] = 2000;
  } else if (ppm[7] < 1200) {
    ppm[7] = 1000;
  } else {
    ppm[7]= 1500;
  }
  
	
	ppm_buffer[5] = ppm[5];     
	
	if (!((ppm[0] == 1500) && (ppm[1] == 1500) &&
		(ppm[2] == 1500) && (ppm[3] == 1500) &&
		(ppm[4] == 1000) && ppm[6] == 1000)) {
		ppm_buffer[0] = ppm[0];
		ppm_buffer[1] = ppm[1];
		ppm_buffer[2] = ppm[2];
		ppm_buffer[3] = ppm[3];
		ppm_buffer[4] = ppm[4];
    ppm_buffer[6] = ppm[6];
    ppm_buffer[7] = ppm[7];
	}
	else {
		counter++;
	}

	if (counter == 10) {
		ppm_buffer[0] = ppm[0];
		ppm_buffer[1] = ppm[1];
		ppm_buffer[2] = ppm[2];
		ppm_buffer[3] = ppm[3];
		ppm_buffer[4] = ppm[4];
    ppm_buffer[6] = ppm[6];
    ppm_buffer[7] = ppm[7];
		counter = 0;
	}
}

uint32_t Frisky::get_ch(int ch)   // ppm_buffer[0] contains Frsky channel 1 data and so on
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
	ppm[4] = 1000;	// AUTONOMOUS
	ppm[5] = 1000;	// 0dB
}
