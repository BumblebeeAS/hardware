#ifndef _ASV2TELEMETRY_h
#define _ASV2TELEMETRY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Wire.h>

#define BATT1 1
#define BATT2 2
#define KILL  3
#define POSB  4
#define POPB  5
#define DAC_addr 0x4C

class board {
private:
	uint32_t prev = 1000000;
	uint32_t delta = 0;

public:
	void init();
	void writeDAC(int voltage);
	void readCPPM(int arr[], )
};

#endif