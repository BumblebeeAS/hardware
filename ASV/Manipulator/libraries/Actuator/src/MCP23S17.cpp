#include "MCP23S17.h"
#include <SPI.h>

/********************************************
*   THE PROGRAM USES BANK 0 FOR OPERATION   *
*   ADDRESS POINTER WORK IN SEQUENTIAL MODE *
*	THE SPI BUS MUST BE INITIALIZED FIRST	*
*********************************************/

/*****************************
*   START OF PRIVATE METHOD  *
******************************/

void MCP23S17::setAddrPtr(uint8_t addr, uint16_t mode) {

	uint16_t opcode16;

	opcode16 = 0b0100000000000000 | (_addr << 9) | addr | (mode << 8);
	SPI.transfer16(opcode16);

}

uint16_t MCP23S17::readReg(MCP23S17Reg reg, ByteType byteType, bool release) {

	uint16_t opcode16;
	uint16_t result = 0x00;

	//select device
	digitalWrite(_cs, LOW);

	if (_release == true) {
		//set address pointer
		setAddrPtr((reg == LOW_BYTE) ? (uint8_t(reg) + 0x01) : (reg), READ);
	}

	if (byteType != WORD) {
		//read one byte of data
		result = SPI.transfer(0xFF);
	}
	else {
		//read one word
		result = SPI.transfer16(0xFFFF);
	}

	_release = release;

	if (release) {
		//release SPI bus
		digitalWrite(_cs, HIGH);
		SPI.endTransaction();
	}

	return result;
}

uint16_t* MCP23S17::readReg(MCP23S17Reg start, MCP23S17Reg end, bool release) {

	uint16_t opcode16;
	static uint16_t result[11];
	uint8_t size = (end - start) / 2;
	uint8_t i = 0;

	//select device
	digitalWrite(_cs, LOW);

	if (_release == true) {
		//set address pointer
		setAddrPtr(start, READ);
	}

	_release = release;

	while (i < size) {
		//read until the number of reg is reached
		result[i++] = SPI.transfer16(0xFFFF);
	}

	if (release) {
		//release SPI bus
		digitalWrite(_cs, HIGH);
		SPI.endTransaction();
	}

	return result;
}

void MCP23S17::writeReg(MCP23S17Reg reg, ByteType byteType, uint16_t data, bool release) {

	uint16_t opcode16;

	//select device
	digitalWrite(_cs, LOW);

	if (_release == true) {
		//set address pointer
		setAddrPtr((reg == LOW_BYTE) ? (uint8_t(reg) + 0x01) : (reg), WRITE);
	}

	_release = release;

	if (byteType != WORD) {
		//write one byte
		SPI.transfer(data);
	}
	else {
		//write one word
		SPI.transfer16(data);
	}

	if (release) {
		//release SPI bus
		digitalWrite(_cs, HIGH);
		SPI.endTransaction();
	}
}

void MCP23S17::writeReg(MCP23S17Reg start, MCP23S17Reg end, const uint8_t* data, bool release) {

	uint16_t opcode16;
	uint8_t size = (end - start) / 2;
	uint8_t i = 0;

	//select device
	digitalWrite(_cs, LOW);

	if (_release == true) {
		//set address pointer
		setAddrPtr(start, WRITE);
	}

	_release = release;

	while (i < size) {
		//write until number of register is reached
		SPI.transfer(data[i++]);
	}

	if (release) {
		//release SPI bus
		digitalWrite(_cs, HIGH);
		SPI.endTransaction();
	}
}

/**************************
*   END OF PUBLIC METHOD  *
***************************/

/****************************
*   START OF PUBLIC METHOD  *
*****************************/

MCP23S17::MCP23S17(uint8_t addr, uint8_t cs_pin) : _addr(addr), _cs(cs_pin), _release(true) {
	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);
}

void MCP23S17::init(void) {
	//more functionality can be added in for init

	//default GPIO direction is input, no pull up, sequential address indexing
	//INT pins not connected together, bank 0
	setIOMode(SEQ_ADDR_ENABLE | SLEW_RATE_CON_ENABLE | MIRROR_ENABLE
			 | ADDR_PIN_DISABLE | INT_OPEN_DRIAN_DISABLE | INT_ACTIVE_HIGH);

	setIODir(0xFFFF);
	setIOPullUp(0x0000);
}

void MCP23S17::setIOMode(uint16_t conData) {

	uint8_t opcode8 = 0;
	//write IOCON register
	for (uint16_t mask = 1; mask <= MIRROR_DISABLE; mask <<= 1) {
		switch (mask & conData) {
		case SEQ_ADDR_ENABLE: opcode8 &= 0xDF; break;
		case SEQ_ADDR_DISABLE: opcode8 |= 0x20; break;
		case SLEW_RATE_CON_ENABLE: opcode8 &= 0xEF; break;
		case SLEW_RATE_CON_DISABLE: opcode8 |= 0x10; break;
		case ADDR_PIN_ENABLE: opcode8 |= 0x08; break;
		case ADDR_PIN_DISABLE: opcode8 &= 0xF7; break;
		case INT_OPEN_DRIAN_ENABLE: opcode8 |= 0x04; break;
		case INT_OPEN_DRIAN_DISABLE: opcode8 &= 0xFB; break;
		case INT_ACTIVE_HIGH: opcode8 |= 0x02; break;
		case INT_ACTIVE_LOW: opcode8 &= 0xFD; break;
		case MIRROR_ENABLE: opcode8 |= 0x40; break;
		case MIRROR_DISABLE: opcode8 &= 0xBF; break;
		default: break;
		}
	}
	writeReg(IOCON, HIGH_BYTE, opcode8, true);
}

void MCP23S17::setIODir(uint16_t IODir) {
	//1 means input, 0 means output
	writeReg(IODIR, WORD, IODir, true);
}

void MCP23S17::setIOPullUp(uint16_t IOPullUp) {
	//1 means pull up, 0 means no pull up
	writeReg(GPPU, WORD, IOPullUp, true);
}

void MCP23S17::writeGPIO(uint8_t pinNum, bool logic) {

	//read from the device to determine the current latch value
	//merge with desire value to produce output
	if (logic == HIGH) {
		writeReg(OLAT, WORD, readReg(OLAT, WORD, true) | (1 << pinNum), true);
	}
	else {
		writeReg(OLAT, WORD, readReg(OLAT, WORD, true) & (~(1 << pinNum)), true);
	}
}

void MCP23S17::writeGPIO(uint16_t pattern) {
	writeReg(OLAT, WORD, pattern, true);
}

uint16_t MCP23S17::readGPIO(void) {
	return readReg(GPIO, WORD, true);
}

uint16_t MCP23S17::readReg(MCP23S17Reg reg){
	return readReg(reg, WORD, true);
}

uint16_t* MCP23S17::readReg(MCP23S17Reg start, MCP23S17Reg end) {
	return readReg(start, end, true);
}

/**************************
*   END OF PUBLIC METHOD  *
***************************/