#include "Sweeper.h"

#define HORIZONTAL 2250    // initial flat position at 157deg  ( 0-180 scale )

/**************************
*	START OF PUBLIC METHOD
***************************/

Sweeper::Sweeper(int interval) {
	updateInterval = interval;
}

void Sweeper::attach(int pin) {
	servo.attach(pin);
	servo.writeMicroseconds(HORIZONTAL);
}

void Sweeper::detach(void) {
	servo.detach();
}

void Sweeper::update(int target_us) { 
	int move = target_us; //map(target_us,0,38,1980,2456);		//angle recevied from 0-38 re-map to µs
													//2456 is max backward-------176 deg-------38
	if (move>2456) {								//2250 is flat---------------157 deg-------19
		move=2456;									//1980 is max forward--------132 deg-------0
	}
	if (move<1980) {
		move=1980;
	}
	if ((millis() - lastUpdate) > updateInterval) {
		lastUpdate = millis();
		servo.writeMicroseconds(move);
	}
}

/**************************
*	END OF PUBLIC METHOD
***************************/