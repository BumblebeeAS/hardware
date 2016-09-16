#ifndef TempAD7414_h
#define TempAD7414_h

#include <Arduino.h>
#include <Wire.h>


class TempAD7414
{
public:
	TempAD7414(int fig, int i2c_address);
	void initTempAD7414(void);
	double getTemp(void);

private:
	int address;
	int configRegister;
	byte lowByteIn;
	byte highByteIn;
	uint16_t temp;
	double final;
};

#endif

