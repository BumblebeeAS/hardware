#ifndef Brushed_H
#define Brushed_H

#include "Arduino.h"
#include "Brushed_def.h"

class Brushed
{
public:
	Brushed(volatile uint8_t directions_Register_a,volatile uint8_t directions_Register_b, volatile uint8_t IO_Register, int SR, int RS, int PWM, int FF1, int FF2, int PWM_pin);
    ~Brushed();
	void init();
    void reset(void);
	void run(int speed);
	void stop(void);
private:
    
    volatile uint8_t Channel;
    int Channel_Reset;
    int PWM_Pin;


};

#endif
