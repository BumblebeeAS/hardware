#ifndef TempAD7414_h
#define TempAD7414_h

#include <Arduino.h>
#include <Wire.h>


class TempAD7414
{
public:
TempAD7414(int pin);
void initTempAD7414(void);
double getTemp(void);

private:
int configRegister;
byte lowByteIn;
byte highByteIn;
uint16_t temp;
double final;
};

#endif

