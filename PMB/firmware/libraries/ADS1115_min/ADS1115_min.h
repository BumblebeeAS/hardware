//ADS_simple.h
#ifndef ADS1115_MIN_H
#define ADS1115_MIN_H

#include <Arduino.h>
#include <Wire.h>

class ADS1115{
private:
	int address;
public:
	ADS1115(int i2c_address);
	void init();
	uint16_t readChannel(uint8_t input);	
};

#endif