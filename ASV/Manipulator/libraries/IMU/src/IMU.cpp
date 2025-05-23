#include "IMU.h"
#include <Wire.h>
#include <math.h>
#include <avr/eeprom.h>

/***********************************************************************************************
*	References for use on imu
*	http://kitsprout.logdown.com/posts/335386  
*	http://www.starlino.com/imu_guide.html
*	http://www.analog.com/media/en/technical-documentation/application-notes/AN-1057.pdf
************************************************************************************************/


/****************************
*	START OF PRIVATE METHOD *
*****************************/

unsigned int IMU::readReg(int reg) {
	Wire.beginTransmission(MPU_addr);
	Wire.write(reg);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_addr, 1, true);
	return Wire.read();
}

void IMU::writeReg(int reg, int data) {
	Wire.beginTransmission(MPU_addr);
	Wire.write(reg);
	Wire.write(data);
	Wire.endTransmission(true);
}

/*************************
*	END OF PRIVATE METHOD*
**************************/

/**************************
*	START OF PUBLIC METHOD*
***************************/

int compare(const double lhs, const double rhs) {
	if(lhs < rhs) {
		return -1;
	}
	else if(lhs == rhs) {
		return 0;
	}
	else {
		return 1;
	}
}

void IMU::init() {
	Wire.begin();
	Wire.setClock(400000L);
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x6B);  // PWR_MGMT_1 register
	Wire.write(0);     // set to zero (wakes up the MPU-6050)
	Wire.endTransmission(true);

	//wait for eeprom to be ready
	while(!eeprom_is_ready());
	//read saved result from eeprom
	zero_pt = eeprom_read_float(0);
	Serial.print("Previously Calibrated Zero: ");
	Serial.println(zero_pt);

	pinMode(9, INPUT);
	if (digitalRead(9) == LOW) {
		correctToZero();
		for (uint8_t i = 0; i < 3; i++) {
			for (uint8_t j = 0; j < 255; j++) {
				Serial.write(255);
			}
			delay(500);
		}
	}
}

IMU::IMU(const int addr) {
	caliAx = 0, caliAy = 0, caliAz = 0, caliGx = 0, caliGy = 0, caliGz = 0;
	MPU_addr = addr;
	error = 0;
}

IMU::IMU(void) {
	caliAx = 0, caliAy = 0, caliAz = 0, caliGx = 0, caliGy = 0, caliGz = 0;
	MPU_addr = 0x68;
	error = 0;
}


void IMU::setting() {
	int gyro_config = readReg(0x1B);                    // check gyro and accel config setting
	int accel_config = readReg(0x1C);                   //  bit4 bit3,
	Serial.print("gyro config: ");
	Serial.print(gyro_config, BIN);                     //gyro=250 500 1000 2000       0x1B
	Serial.println("0=250, 01=500, 10=1000, 11=2000 degree/s");
	Serial.print("accel config: ");
	Serial.print(accel_config, BIN);                    //accel= 2 4 8 16              0x1C
	Serial.println("0=2, 01=4, 10=8, 11=16 g");
}

void IMU::readAccTempGyro() {
	Wire.beginTransmission(MPU_addr);
	Wire.write(0x3B);     // starting with register 0x3B (ACCEL_XOUT_H)
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers

	AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
	AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
	AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
	Tmp = Wire.read() << 8 | Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
	GyX = Wire.read() << 8 | Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
	GyY = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
	GyZ = Wire.read() << 8 | Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

	AcX -= 1726;  AcY -= 146; AcZ -= 930; GyX += 221; GyY -= 51;  GyZ = GyZ;
}

void IMU::correctToZero() {
	double zeros[9] = {0};
	for(int i = 0; i < 9; i ++) {
		readAccTempGyro();
		zeros[i] = atan2(AcX, AcZ) * 180 / PI;
	}
	qsort(zeros, 9, sizeof(double), compare);
	//take the median value
	zero_pt = zeros[4];
	Serial.print("Calibrated Zero Pitch: ");
	Serial.println(zero_pt);

	//wait for eeprom ready
	while(!eeprom_is_ready());
	//save result into eeprom at first location
	eeprom_update_float(0, zero_pt);
}

double IMU::calculateError() {
	readAccTempGyro();
	return zero_pt - atan2(AcX, AcZ) * 180 / PI;
}

void IMU::printAccTempGyro() {
	Serial.println("Parameters: ");
	Serial.print("AcX = "); Serial.println(AcX);
	Serial.print("AcY = "); Serial.println(AcY);
	Serial.print("AcZ = "); Serial.println(AcZ);
	Serial.print("Tmp = "); Serial.println(Tmp / 340.00 + 36.53);  //equation for temperature in degrees C from datasheet
	Serial.print("GyX = "); Serial.println(GyX);
	Serial.print("GyY = "); Serial.println(GyY);
	Serial.print("GyZ = "); Serial.println(GyZ);
}

void IMU::calibrateAcc() {
	for (int i = 0; i<100; i++) {
		readAccTempGyro();
		caliAx += AcX;            //here provides the calibration for IMU's accelerometer
		caliAy += AcY;            //by placing the accelerometer in different orientation 
		caliAz += AcZ;            //to obtain 1g in the direction
		caliGx += GyX;
		caliGy += GyY;
		caliGz += GyZ;
	}
	caliAx /= 100;
	caliAy /= 100;
	caliAz /= 100;
	caliGx /= 100;
	caliGy /= 100;
	caliGz /= 100;
	Serial.println();
	Serial.println(caliAx);
	Serial.println(caliAy);
	Serial.println(caliAz);
	Serial.println(caliGx);
	Serial.println(caliGy);
	Serial.println(caliGz);
	delay(3000);
}

/**************************
*	END OF PUBLIC METHOD  *
***************************/