#ifndef _FRISKY_
#define _FRISKY_

#include <Arduino.h>

class Frisky
{
private:
	static int ppm[6]; 

	int _cppm_pin; // pin in for cppm

	static uint32_t prev;
	static uint32_t delta;
	static bool sync;
	static int _ch;

	static void readppm();

public:
	Frisky(int cppm_pin);
	void init();

	uint32_t get_ch(int ch);
	uint32_t get_last_int_time();
	void reset();
};

#endif