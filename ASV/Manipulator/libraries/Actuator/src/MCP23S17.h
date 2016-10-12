// MCP23S08.h
// initialize SPI of Arduino before using this library

#ifndef _MCP23S08_h
#define _MCP23S08_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//definition of device parameters
enum MCP23S17Reg : uint8_t {
	//the enum is defined to match bank 0 register address
	IODIR = 0x00,
	IPOL = 0x02,
	GPINTEN = 0x04,
	DEFVAL = 0x06,
	INTCON = 0x08,
	IOCON = 0x0A,
	GPPU = 0x0C,
	INTF = 0x0E,
	INTCAP = 0x10,
	GPIO = 0x12,
	OLAT = 0x14,
};

enum ByteType : uint8_t {
	HIGH_BYTE = 0x00,
	LOW_BYTE,
	WORD
};

#define WRITE						0x0000
#define READ						0x0001

#define SEQ_ADDR_ENABLE				0x0001
#define SEQ_ADDR_DISABLE			0x0002
#define SLEW_RATE_CON_ENABLE		0x0004
#define SLEW_RATE_CON_DISABLE		0x0008
#define ADDR_PIN_ENABLE				0x0010
#define ADDR_PIN_DISABLE			0x0020
#define INT_OPEN_DRIAN_ENABLE		0x0040
#define INT_OPEN_DRIAN_DISABLE		0x0080
#define INT_ACTIVE_HIGH				0x0100
#define INT_ACTIVE_LOW				0x0200
#define MIRROR_ENABLE				0x0400
#define MIRROR_DISABLE				0x0800

class MCP23S17
{
 private:
	 uint8_t _addr;
	 uint8_t _cs;
	 bool _release;

	 //set device address pointer location
	 void setAddrPtr(uint8_t addr, uint16_t mode);

	 //read device register
	 uint16_t readReg(MCP23S17Reg reg, ByteType byteType, bool release);
	 uint16_t* readReg(MCP23S17Reg start, MCP23S17Reg end, bool release);

	 //write device register
	 void writeReg(MCP23S17Reg reg, ByteType byteType, uint16_t data, bool release);
	 void writeReg(MCP23S17Reg start, MCP23S17Reg end, const uint8_t* data, bool release);

 public:
	 //constructor
	 MCP23S17(uint8_t addr, uint8_t cs_pin);

	 //init MCP23S17
	 void init(void);

	 //config general I/O operation
	 void setIOMode(uint16_t conData);
	 void setIODir(uint16_t IODir);
	 void setIOPullUp(uint16_t IOPullUp);

	 //read actual GPIO pin values
	 uint16_t readGPIO(void);

	 //read register value
	 uint16_t readReg(MCP23S17Reg reg);
	 uint16_t* readReg(MCP23S17Reg start, MCP23S17Reg end);

	 //write GPIO value
	 void writeGPIO(uint8_t pinNum, bool logic);
	 void writeGPIO(uint16_t pattern);

};

#endif

