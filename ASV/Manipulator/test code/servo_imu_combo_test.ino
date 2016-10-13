#include <Servo.h>
#include <Wire.h>
#include "IMU.h"
#include "Sweeper.h"

long lastMillis = millis();
long interval = 1000;

Sweeper sweep(1);					//init servo object
IMU imu;							// I2C address of the MPU-6050 is pre-set 0x68 if not given

void setup(){
  sweep.attach(9);                 // pin servo is attached
  Serial.begin(115200);
  imu.correctToZero();
}
void loop(){

	imu.readAccTempGyro();
	
	if (millis() - lastMillis > interval) {
		lastMillis = millis();
		imu.printAccTempGyro();
	}

	int target_us = 0;
	int diff = (round)(975 / 90 * imu.correctError() + 1525);

	sweep.update(target_us, diff);

}

