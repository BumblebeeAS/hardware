#ifndef LEDS_h
#define LEDS_h

#include <Arduino.h>


class LEDS
{
public:
LEDS(int pin1,int pin2,int pin3);
void colour(int colour);
void setcolour(int pin1,int pin2,int pin3);

private:
int RedPin,GreenPin,BluePin;
//void getcolour(int pin1,int pin2,int pin3);

};

#endif