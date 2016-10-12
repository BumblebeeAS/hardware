#include <Actuator.h>
#include <TimerOne.h>

Actuator* actPtr = nullptr;

void actuatorCallBackWrapper(void) {
	actPtr->actuatorCallBack();
}

void Actuator::actuatorCallBack(void) {

	static bool active;
		
	active = false;
	
	if (rotary_deploy.requestToDisconnect) {
		active = true;
		if (++rotary_deploy.currTime >= rotary_deploy.time) {
			rotary_deploy.requestToDisconnect = false;
			mcp23s17.writeGPIO(rotary_deploy.pin, LOW);
			if (linear.requestToDisconnect) {
				Serial.println("getcall");
				mcp23s17.writeGPIO(linear.pin, HIGH);
				linear.requestToDisconnect = false;
			}
		}
	}
	else if (linear.requestToDisconnect) {
		active = true;
		if (++linear.currTime >= linear.time) {
			mcp23s17.writeGPIO(linear.pin, LOW);
			linear.requestToDisconnect = false;
			if (rotary_retrieve.requestToDisconnect) {
				mcp23s17.writeGPIO(rotary_deploy.pin, LOW);
				mcp23s17.writeGPIO(rotary_retrieve.pin, HIGH);
			}
		}
	}
	else if (rotary_retrieve.requestToDisconnect) {
		active = true;
		if (++rotary_retrieve.currTime >= rotary_retrieve.time) {
			mcp23s17.writeGPIO(rotary_retrieve.pin, LOW);
			rotary_retrieve.requestToDisconnect = false;
		}
	}
	
	for (uint8_t i = 0; i < 4; i++) {
		if (shooter[i].requestToDisconnect) {
			active = true;
			if (++shooter[i].currTime >= shooter[i].time) {
				mcp23s17.writeGPIO(shooter[i].pin, LOW);
				shooter[i].requestToDisconnect = false;
			}
		}
	}

	if (!active) {
		Timer1.stop();
	}
}

Actuator::Actuator(uint8_t rotary_deploy, uint8_t rotary_retrieve, uint8_t pin_linear, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t pin_s4) {
	this->rotary_deploy.pin = rotary_deploy;
	this->rotary_retrieve.pin = rotary_retrieve;
	this->linear.pin = pin_linear;
	this->shooter[0].pin = pin_s1;
	this->shooter[1].pin = pin_s2;
	this->shooter[2].pin = pin_s3;
	this->shooter[3].pin = pin_s4;
	
	actuationStatus = REST;
}

Actuator::Actuator(void){
	this->rotary_deploy.pin = 0x01;
	this->rotary_retrieve.pin = 0x03;
	this->linear.pin = 0x02;
	this->shooter[0].pin = 0x10;
	this->shooter[1].pin = 0x20;
	this->shooter[2].pin = 0x40;
	this->shooter[3].pin = 0x80;
	
	actuationStatus = REST;
}

void Actuator::init(void) {

	mcp23s17.init();

	//set all IO direction as output
	mcp23s17.setIODir(0x0000);
	mcp23s17.writeGPIO(0x0000);

	//initialize the actuation time to their default value
	rotary_deploy.time = ROTARY_DEFAULT;
	rotary_retrieve.time = 100;
	linear.time = LINEAR_DEFAULT;
	shooter[0].time = SHOOTER_DEFAULT;
	shooter[1].time = SHOOTER_DEFAULT;
	shooter[2].time = SHOOTER_DEFAULT;
	shooter[3].time = SHOOTER_DEFAULT;

	actPtr = this;

	//initialize timer1
	Timer1.initialize(100000);
	//attach call back function
	Timer1.attachInterrupt(actuatorCallBackWrapper);
	Timer1.stop();

}

void Actuator::deployAcoustic(ActuationType actuationType) {
	if (actuationStatus == REST) {
		if (actuationType == ROTARY) {
			//turn off the opposite side first
			mcp23s17.writeGPIO(rotary_retrieve.pin, LOW);
			mcp23s17.writeGPIO(rotary_deploy.pin, HIGH);
			//update actuation status
			actuationStatus = ROTATED;
			rotary_deploy.currTime = 0;
			rotary_retrieve.requestToDisconnect = false;
			rotary_deploy.requestToDisconnect = true;
			Timer1.restart();
		}
	}
	else if (actuationStatus == ROTATED) {
		if (actuationType == LINEAR) {
			actuationStatus = ROTATE_EXTENDED;
			linear.currTime = 0;
			linear.requestToDisconnect = true;
			if (!rotary_deploy.requestToDisconnect) {
				mcp23s17.writeGPIO(linear.pin, HIGH);
				linear.requestToDisconnect = false;
			}
		}
	}

}

void Actuator::retrieveAcoutic(ActuationType actuationType) {
	if (actuationStatus == ROTATE_EXTENDED) {
		if (actuationType == LINEAR) {
			mcp23s17.writeGPIO(linear.pin, LOW);
			linear.currTime = 0;
			linear.requestToDisconnect = true;
			Timer1.restart();
			actuationStatus = ROTATED;
		}
	}
	else if (actuationStatus == ROTATED) {
		if (actuationType == ROTARY) {
			actuationStatus = REST;
			rotary_retrieve.currTime = 0;
			rotary_deploy.requestToDisconnect = false;
			rotary_retrieve.requestToDisconnect = true;
			if (!linear.requestToDisconnect) {
				mcp23s17.writeGPIO(rotary_deploy.pin, LOW);
				mcp23s17.writeGPIO(rotary_retrieve.pin, HIGH);
				Timer1.restart();
			}
		}
	}
}

void Actuator::actuateShooter(uint8_t num) {

}
