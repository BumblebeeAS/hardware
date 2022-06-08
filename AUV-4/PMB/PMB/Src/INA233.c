#include "INA233.h"

//Constructors

void INA233(uint16_t maxCurrent, uint8_t shuntValue, uint8_t addr){
	_maxCurrent = maxCurrent;
	_shuntValue = shuntValue;
	_currLsb = 1000000.0 * _maxCurrent / 32768;
	_addr = addr;
}


void INA_Init(){
	INA233(MAX_CURRENT, SHUNT_RES, INA233_ADDRESS);
	INA_Calibrate();
}

void INA_Calibrate(){
	//Set calibration value
	int calValue = 0.00512 * 32768 / _maxCurrent / _shuntValue * 1000;
	uint8_t numByte = 2;

	INA_ReadRegister(_addr, I2C_CMD_CALIB, numByte);
	INA_dataW[1] = INA_dataR[0] & 0x80;
	INA_dataW[2] = INA_dataR[1] & 0x01;
	calValue = calValue << 1;
	INA_dataW[1] |= calValue >> 8;
	INA_dataW[2] |= calValue;
	INA_WriteRegister(_addr, I2C_CMD_CALIB, numByte);

	//Set Energy Accumulator auto-clear after read
	numByte = 1;
	INA_ReadRegister(_addr, I2C_CMD_CONFIG, numByte);
	INA_dataW[0] = INA_dataR[0] | 0x04; //sets READ_EIN autoclear bit
	INA_WriteRegister(_addr, I2C_CMD_CONFIG, numByte);
}

//Utilities
void INA_ClrBuffer(){
	for (int i=0; i<8; i++){
		INA_dataR[i] = 0;
	}
}

void INA_WriteRegister(uint8_t addr, uint8_t cmd, uint8_t numByteToSend){
	INA_dataW[0] = cmd;
	HAL_I2C_Master_Transmit(&hi2c1, INA233_ADDRESS, INA_dataW, numByteToSend+1, 10);
}

void INA_ReadRegister(uint8_t addr, uint8_t cmd, uint8_t numByteToReq){
	INA_ClrBuffer();
	INA_dataW[0] = (uint8_t)cmd;
	HAL_I2C_Master_Transmit(&hi2c1, INA233_ADDRESS, INA_dataW, 1, 10);
	HAL_Delay(10);
	HAL_I2C_Master_Receive(&hi2c1, INA233_ADDRESS, &INA_dataR[0], numByteToReq, 10);
}


//Getters
uint16_t INA_ReadVoltage(){
	uint8_t numByte = 2;
	INA_ReadRegister(_addr, I2C_CMD_VOLTAGE, numByte);
	uint16_t voltage = (( INA_dataR[0] << 8) | INA_dataR[1]) >> 3; //increments of 10mV (Resolution is 1.25mV/bit)
	return voltage;
}

int16_t INA_ReadShuntVoltage(){
	uint8_t numByte = 2;
	INA_ReadRegister(_addr, I2C_CMD_SHUNTVOLT, numByte);
	int16_t shuntVolt = ((INA_dataR[0] << 8) | INA_dataR[1]) >> 2; //increments of 10uV (Resolution is 2.5uV/bit)
	return shuntVolt;
}

int16_t INA_ReadCurrent(){
	uint8_t numByte = 2;
	INA_ReadRegister(_addr, I2C_CMD_CURRENT, numByte);
	int16_t curr = ((INA_dataR[0] << 8) | INA_dataR[1]);
	return curr;
}

float INA_FloatCurrent(int16_t bCurrent){
	return _currLsb * bCurrent / 2000000.0; //Resolution 4.5mA
}

void INA_ClearEnergyAcc() {
	INA_ClrBuffer();
	uint8_t numByte = 0;
	INA_WriteRegister(_addr, I2C_CMD_CLEAREIN, numByte);
}

/*
 * Default sampling time is 1.1ms
 */
uint32_t INA_ReadEnergy(){
	uint8_t numByte = 7;
	INA_ReadRegister(_addr, I2C_CMD_READEIN, numByte);
//	INA_dataW[0] - Sample count high byte
//	INA_dataW[1] - Sample count mid byte
//	INA_dataW[2] - Sample count low byte
//	INA_dataW[3] - Power accumulator overflow
//  INA_dataW[4] - Power acc high byte
//  INA_dataW[5] - Power acc low byte
//	Serial.print("START:  ");
//	Serial.print(INA_dataW[4]); Serial.print("  |  ");
//	Serial.print(INA_dataW[5]); Serial.print("  |  ");
//	Serial.println(INA_dataW[4] << 8 | INA_dataW[5]);
	return INA_dataR[4] << 8 | INA_dataR[5]; // INA_dataW[4] - High byte, INA_dataW[5] low byte of power accumulator
	//Default sampling time is 1.1ms
}
