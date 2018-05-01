#ifndef LEDS_h
#define LEDS_h

#include <Arduino.h>

class LEDS
{
private:
	int RedPin, GreenPin, BluePin;
	uint32_t time;
	int blink_on;

public:
	LEDS(int pin1,int pin2,int pin3);
	void colour(int colour);
	void setcolour(int pin1,int pin2,int pin3);
	void blink(uint8_t a, uint8_t b, uint32_t period);
};

#endif