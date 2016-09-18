#include <Servo.h>
#include <Wire.h>
#include "IMU.h"
#include "Sweeper.h"

Sweeper sweep(5);
IMU imu(0x68);

void setup(){
  Serial.begin(115200);
  imu.init();
  imu.correctToZero();          // run only when fitting to new position
  sweep.attach(9);             // pin connection
}

void loop(){
  
  int target_us = 0,diff = 0;
 
  imu.readAccTempGyro();
  diff = (round)(975 / 90 * imu.correctError() + 1525);
  sweep.update(target_us, diff);
}



