//ADS_simple.h
#ifndef ADS1115_h
#define ADS1115_h

#include <Arduino.h>
#include <Wire.h>


class ADS1115{
public:
	ADS1115(int pin);
	void init();
	uint16_t readChannel(uint8_t input);	

private:
	int address;
};

#endif