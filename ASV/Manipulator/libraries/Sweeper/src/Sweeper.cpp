#include "Sweeper.h"

#define HORIZONTAL 2168    // initial flat position at 15deg (0-38deg scale)


/**************************
*	START OF PUBLIC METHOD
***************************/

Sweeper::Sweeper(int interval) {
	updateInterval = interval;

}

void Sweeper::attach(int pin) {
	servo.attach(pin);
	target = HORIZONTAL;
}

void Sweeper::detach(void) {
	servo.detach();
}

void Sweeper::update(int move) { 
	if (enable != 1) {
		return;
	}
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