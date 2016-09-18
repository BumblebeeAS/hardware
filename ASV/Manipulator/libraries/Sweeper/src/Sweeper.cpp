#include "Sweeper.h"

/**************************
*	START OF PUBLIC METHOD
***************************/

Sweeper::Sweeper(int interval) {
	updateInterval = interval;
}

void Sweeper::attach(int pin) {
	servo.attach(pin);
}

void Sweeper::detach(void) {
	servo.detach();
}

void Sweeper::update(int target_us, int error) {
	if ((millis() - lastUpdate) > updateInterval) {
		lastUpdate = millis();
		servo.writeMicroseconds(target_us + error);
	}
}

/**************************
*	END OF PUBLIC METHOD
***************************/