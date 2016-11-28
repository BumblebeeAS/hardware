#include <Actuator.h>
#include "FlexiTimer2\FlexiTimer2.h"

//create an actuator pointer for call back function
Actuator* actPtr = nullptr;

void actuatorCallBackWrapper(void) {
	//point the pointer to the actual actuator call back function
	actPtr->actuatorCallBack();
}

void Actuator::actuatorCallBack(void) {

	bool stopTimer;

	//check all shooters if a prescribed delay has passed
	for (uint8_t i = 0; i < 4; i++) {
		if (shooter[i].requestToDisconnect == true) {
			shooter[i].currTime++;
			if (shooter[i].currTime == shooter[i].time) {
				shooter[i].requestToDisconnect = false;
				mcp23s17.writeGPIO(shooter[i].pin, LOW);
			}
		}
	}

	//if none of the shooter is actuating, turn off timer 1
	for (uint8_t i = 0; i < 4; i++) {
		if (shooter[i].requestToDisconnect == true) {
			stopTimer = false;
			break;
		}
		else {
			stopTimer = true;
		}
	}

	if(stopTimer == true){
		FlexiTimer2::stop();
	}

}

Actuator::Actuator(uint8_t rotateDown, uint8_t rotateUp, uint8_t pin_linear, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t pin_s4) {
	this->rotateDown.pin = rotateDown;
	this->rotateUp.pin = rotateUp;
	this->linear.pin = pin_linear;
	this->shooter[0].pin = pin_s1;
	this->shooter[1].pin = pin_s2;
	this->shooter[2].pin = pin_s3;
	this->shooter[3].pin = pin_s4;
}

Actuator::Actuator(void){
	this->rotateDown.pin = 0x01;
	this->linear.pin = 0x02;
	this->rotateUp.pin = 0x03;
	this->shooter[0].pin = 0x04;
	this->shooter[1].pin = 0x05;
	this->shooter[2].pin = 0x06;
	this->shooter[3].pin = 0x07;
}

void Actuator::init(void) {

	mcp23s17.init();

	//set all IO direction as output and output low
	mcp23s17.setIODir(0x0000);
	mcp23s17.writeGPIO(0x0000);

	//initialize the actuation time to their default value
	rotateDown.time = ROTARY_DEFAULT;
	//retrieve is much faster than actuation
	rotateUp.time = 100;
	linear.time = LINEAR_DEFAULT;
	//only shooter delay time is used in the program
	shooter[0].time = SHOOTER_DEFAULT;
	shooter[1].time = SHOOTER_DEFAULT;
	shooter[2].time = SHOOTER_DEFAULT;
	shooter[3].time = SHOOTER_DEFAULT;

	//point the actuator pointer to this object so to use the call back function
	actPtr = this;

	//initialize timer2, interrupt at 100ms, attach call back function
	FlexiTimer2::set(100, actuatorCallBackWrapper);
	//set timer2 to initially stopped
	FlexiTimer2::stop();
}

void Actuator::actuateAcoustic(ActuationType actuationType) {
	//actuate corresponding actuator based on the type specified
	if (actuationType == ROTATE_DOWN) {
		mcp23s17.writeGPIO(rotateDown.pin, HIGH);
	}
	else if (actuationType == ROTATE_UP) {
		mcp23s17.writeGPIO(rotateUp.pin, HIGH);
	}
	else if (actuationType == LINEAR) {
		mcp23s17.writeGPIO(linear.pin, HIGH);
	}
}

void Actuator::stopActuation(ActuationType actuationType) {
	//turn off corresponding actuator based on the type specified
	if (actuationType == ROTATE_DOWN) {
		mcp23s17.writeGPIO(rotateDown.pin, LOW);
	}
	else if (actuationType == ROTATE_UP) {
		mcp23s17.writeGPIO(rotateUp.pin, LOW);
	}
	else if (actuationType == LINEAR) {
		mcp23s17.writeGPIO(linear.pin, LOW);
	}
}

void Actuator::actuateShooter(uint8_t num) {
	bool resetTimer = true;

	//check if the number of shooter is correct
	if (num > 0 && num < 5) {
		//the array index is one number off than the num specified
		mcp23s17.writeGPIO(shooter[num - 1].pin, HIGH);
		//reset actuation to 0
		shooter[num - 1].currTime = 0;
		//set flag to stop actuation after specified time
		shooter[num - 1].requestToDisconnect = true;
		//check if the rest of the shooter is requsting to disconnect
		for (uint8_t i = 1; i <= 4; i++) {
			if (num != i) {
				if (shooter[i - 1].requestToDisconnect == true) {
					resetTimer = false;
					break;
				}
			}
		}

		//if none of the shooter is currently using the timer, start the timer
		if (resetTimer == true) {
			FlexiTimer2::start();
		}
	}
}
