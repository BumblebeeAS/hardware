#include "Sweeper.h"

#define HORIZONTAL 2168    // initial flat position at 15deg (0-38deg scale)
#define MAX_PULSE_LIMIT  2456
#define MIN_PULSE_LIMIT  1980
#define MAX_ANGLE  38
#define MIN_ANGLE  0

/**************************
*	START OF PUBLIC METHOD
***************************/

void Sweeper::attach(int pin) {
	servo.attach(pin, 500, 2500);
	this->target = HORIZONTAL;
}

void Sweeper::detach(void) {
	servo.detach();
}

int Sweeper::currTarget(void) {
	return this->target;
}

void Sweeper::updateTargetPos(int target) {
	this->target = target;
}

void Sweeper::updateCurrPos(int newTarget) {
	//convert newTarget in angles to actual microseconds
	//2456 is max backward-------176 deg-------38
	//2250 is flat---------------157 deg-------19
	//1980 is max forward--------132 deg-------0
	newTarget = map(newTarget, MIN_ANGLE, MAX_ANGLE, MIN_PULSE_LIMIT, MAX_PULSE_LIMIT);
	newTarget = constrain(newTarget, MIN_PULSE_LIMIT, MAX_PULSE_LIMIT);

	servo.writeMicroseconds(newTarget);
}

/**************************
*	END OF PUBLIC METHOD
***************************/