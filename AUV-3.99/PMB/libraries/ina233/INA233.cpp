#include "INA233.h"
#include "Arduino.h"

//Constructors

INA233::INA233(uint16_t maxCurrent, uint8_t shuntValue){
	_maxCurrent = maxCurrent;
	_shuntValue = shuntValue;
	_currLsb = 1000000.0 * _maxCurrent / 32768;
	_addr = 0x45;
}

INA233::INA233(uint16_t maxCurrent, uint8_t shuntValue, uint8_t addr) {
	_maxCurrent = maxCurrent;
	_shuntValue = shuntValue;
	_currLsb = 1000000.0 * _maxCurrent / 32768;
	_addr = addr;
}

void INA233::init(){
	calibrate();
}

void INA233::calibrate(){
	//Set calibration value
	uint16_t calValue = 0.00512 * 32768 / _maxCurrent / _shuntValue * 1000;
	uint8_t numByte = 2;
	read_I2C(_addr, I2C_CMD_CALIB, numByte);
	_buff[0] = _buff[0] & 0x80;
	_buff[1] = _buff[1] & 0x01;
	calValue = calValue << 1;
	_buff[0] |= calValue >> 8;
	_buff[1] |= calValue;
	write_I2C(_addr, I2C_CMD_CALIB, numByte);

	//Set Energy Accumulator auto-clear after read
	numByte = 1;
	read_I2C(_addr, I2C_CMD_CONFIG, numByte);
	_buff[0] = _buff[0] | 0x04; //sets READ_EIN autoclear bit
	write_I2C(_addr, I2C_CMD_CONFIG, numByte);
}

//Utilities
void INA233::clrBuffer(){
	for (int i=0; i<8; i++){
		_buff[i] = 0;
	}
}

void INA233::write_I2C(uint8_t addr, uint8_t cmd, uint8_t numByteToSend){
	Wire.beginTransmission(addr); // start transmission
	Wire.write(cmd); // send command

	//Send LSByte first
	for (int i = numByteToSend; i > 0; i--){
		Wire.write(_buff[i-1]);
	}

	Wire.endTransmission();
}

void INA233::read_I2C(uint8_t addr, uint8_t cmd, uint8_t numByteToReq){
	clrBuffer();
	Wire.beginTransmission(addr); // start transmission
	Wire.write(cmd); // send command
	Wire.endTransmission(false);
	uint8_t a = Wire.requestFrom(addr, numByteToReq);
	while (a > 0 && Wire.available()) {
		_buff[a-1] = Wire.read();
		a--;
	}
	Wire.endTransmission();
}


//Getters
uint16_t INA233::readVoltage(){
	uint8_t numByte = 2;
	read_I2C(_addr, I2C_CMD_VOLTAGE, numByte);
	uint16_t voltage = (( _buff[0] << 8) | _buff[1]) >> 3; //increments of 10mV (Resolution is 1.25mV/bit)
	return voltage;
}

int16_t INA233::readShuntVoltage(){
	uint8_t numByte = 2;
	read_I2C(_addr, I2C_CMD_SHUNTVOLT, numByte);
	int16_t shuntVolt = ((_buff[0] << 8) | _buff[1]) >> 2; //increments of 10uV (Resolution is 2.5uV/bit)
	return shuntVolt;
}

int16_t INA233::readCurrent(){
	uint8_t numByte = 2;
	read_I2C(_addr, I2C_CMD_CURRENT, numByte);
	int16_t curr = ((_buff[0] << 8) | _buff[1]);
	return curr;
}

float INA233::floatCurrent(int16_t bCurrent){
	return _currLsb * bCurrent / 2000000.0; //Resolution 4.5mA
}

void INA233::clearEnergyAcc() {
	clrBuffer();
	uint8_t numByte = 0;
	write_I2C(_addr, I2C_CMD_CLEAREIN, numByte);
}

/*
 * Default sampling time is 1.1ms
 */
uint32_t INA233::readEnergy(){
	uint8_t numByte = 7;
	read_I2C(_addr, I2C_CMD_READEIN, numByte);
//	_buff[0] - Sample count high byte
//	_buff[1] - Sample count mid byte
//	_buff[2] - Sample count low byte
//	_buff[3] - Power accumulator overflow
//  _buff[4] - Power acc high byte
//  _buff[5] - Power acc low byte
//	Serial.print("START:  ");
//	Serial.print(_buff[4]); Serial.print("  |  ");
//	Serial.print(_buff[5]); Serial.print("  |  ");
//	Serial.println(_buff[4] << 8 | _buff[5]);
	return _buff[4] << 8 | _buff[5]; // _buff[4] - High byte, _buff[5] low byte of power accumulator
	//Default sampling time is 1.1ms
}
