#ifndef _FRISKY_
#define _FRISKY_

#include <Arduino.h>

class Frisky
{
private:
	static int ppm[8]; 
	static int ppm_buffer[8];

	int _cppm_pin; // pin in for cppm

	static uint32_t prev;
	static uint32_t delta;
	static bool sync;
	static int _ch;
	static uint8_t counter;

	static void readppm();

	static void checkppm();

public:
	Frisky(int cppm_pin);
	void init();

	uint32_t get_ch(int ch);
	uint32_t get_last_int_time();
	void reset();
};

#endif
