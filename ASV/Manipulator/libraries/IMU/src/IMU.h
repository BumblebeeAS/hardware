#ifndef _IMU_H
#define _IMU_H

#include <Arduino.h>

class IMU {
private:
	int MPU_addr;  // I2C address of the MPU-6050  
																		 //            z      
	int AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;                               //            ^    y																										+90deg
	unsigned long previousMillis;                                        //            |  /                        |																   //            | /                         |
	long caliAx, caliAy, caliAz, caliGx, caliGy, caliGz;             	 //            |------>  x           0deg  ------------------- > x
	double pitch;                                                        //                                        |
	double zero_pt;                                                      //                                        |
	double error;                                                        //																															-90deg

	void writeReg(int reg, int data);    // write value into register
	unsigned int readReg(int reg);            // read value from register

public:
	IMU(int addr);

	void init();				// wake up IMU
	void setting();				// show current acc gyro range setting

	void readAccTempGyro();		// update value from IMU
	void printAccTempGyro();	// print on Serial Monitor

	void correctToZero();		// setting zero_pt
	double correctError();		// correction
	void calibrateAcc();		// loop for ave value of readings

};     // end of IMU class

#endif

