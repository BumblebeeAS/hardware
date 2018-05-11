/*
 * I2C interface for INA233 Current and Power Monitor IC
 * Library written by Zhi Jie, 2017
 */

#ifndef INA233_H
#define INA233_H

#define ARRAY_SIZE 10

#define I2C_CMD_CALIB 0xD4
#define I2C_CMD_CONFIG 0xD5
#define I2C_CMD_VOLTAGE 0x88
#define I2C_CMD_CURRENT 0x89
#define I2C_CMD_SHUNTVOLT 0xD1
#define I2C_CMD_CLEAREIN 0xD6
#define I2C_CMD_READEIN 0x86


#include "Arduino.h"
#include <Wire.h>

class INA233 {

private:
	uint16_t _maxCurrent;
	uint8_t _shuntValue;
	uint8_t _addr;
	float _currLsb;

	uint16_t _voltage = 0;
	int16_t _shuntVoltage = 0;
	int16_t _current = 0;

	uint8_t _buff[8] = { 0 };

	//Methods
	void calibrate();

	//Utilities

	void clrBuffer();
	void write_I2C(uint8_t addr, uint8_t cmd, uint8_t numByteToSend);
	void read_I2C(uint8_t addr, uint8_t cmd, uint8_t numByteToReq);

public:
	//Constructors
	INA233(uint16_t maxCurrent, uint8_t shuntValue);

	INA233(uint16_t maxCurrent, uint8_t shuntValue, uint8_t addr);

	//Initialise
	void init();

	//Getters
	uint16_t readVoltage();
	int16_t readShuntVoltage();
	int16_t readCurrent();
	float floatCurrent(int16_t bCurrent);
	void clearEnergyAcc();
	uint32_t readEnergy();

};

#endif
