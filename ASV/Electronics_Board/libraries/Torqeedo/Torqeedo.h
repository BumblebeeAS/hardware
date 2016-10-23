#pragma once
#ifndef _TORQEEDO_
#define _TORQEEDO_

#include"Arduino.h"

//#define _TEST_

#define MAX_PACKET_SIZE 40 //in bytes
#define END_MARKER 0xF6

#define TORQEEDO1_RXEN 22
#define TORQEEDO1_DXEN 24
#define TORQEEDO2_RXEN 26
#define TORQEEDO2_DXEN 28
#define TORQEEDO1_ON 40
#define TORQEEDO2_ON 38

#define BAUDRATE 19200

/*=======================================================
				PACKET DELIMITERS
	-------------------------------------------------------*/
	#define PACKET_START 0xAC /* marks every packet start */
	#define PACKET_END 0xAD /* marks every packet end */
	#define PACKET_ESCAPE 0xAE /* marks escaped bytes in packet body */
	#define PACKET_ESCAPE_MASK 0x80 /* xor with escaped bytes in packet body */
	//TODO: Handle escaped bytes

/*=======================================================
				DEVICE ADDRESSES
	-------------------------------------------------------*/
// Only valid for master
//Address when slave replies is 0x00
	#define DeviceId_Master	0x00
	#define DeviceId_Remote1	0x14
	#define DeviceId_Display	0x20

/*=======================================================
				MESSAGE ID
				foo sucks donkey dick
	-------------------------------------------------------*/

	/** REMOTE **/
	#define REMOTE_Remote 0x01
	#define REMOTE_SetUp 0x02

	/** DISPLAY**/
	#define DISPLAY_SystemState	0x41
	#define DISPLAY_SystemSetup	0x42

	void write1(byte content);
	byte read1();
	int available1();
	void flush1();
	void write2(byte content);
	byte read2();
	int available2();
	void flush2();
	
	typedef struct {
		int16_t motor_speed;
		byte body[6];
	} MotorDrive;
	
typedef struct {
	// Some random flags. Ignore first lolol
	uint16_t motor_voltage;
	uint16_t motor_current;
	uint16_t motor_power;
	int16_t motor_speed;
	uint8_t battery_charge;
	uint16_t battery_voltage;
	uint16_t battery_current;
} DisplayState;

class Torqeedo
{
private:
	int RX_ENABLE;
	int DX_ENABLE;
	int ON_PIN;
	int thrusterNum;
	unsigned long time;

	MotorDrive _motorDrive;
	DisplayState _displayState;

	uint8_t motorstats[8];
	uint8_t batterystats[6];
	uint8_t rangestats[6];

	bool msgStart  = false;
	byte data[MAX_PACKET_SIZE];
	int len = 0; //length of packet body including header
	int _checksum;
	int16_t hardcodespeed = 0;
	bool speedZero = true;
	
	typedef void(*WriteCallback)  (const byte what);    // send a byte to serial port
	typedef int(*AvailableCallback)  ();    // return number of bytes available
	typedef byte(*ReadCallback)  ();    // read a byte from serial port
	typedef void(*FlushCallback)  ();    // read a byte from serial port

	WriteCallback _write;
	AvailableCallback _available;
	ReadCallback _read;
	FlushCallback _flush;

public:
	int startUpCount = 0;
	bool kill = false;

	Torqeedo(int RXEN, int DXEN, int ON, int thruster_num);
	~Torqeedo();
	void init();

	bool setMotorDrive(int speed);
	uint8_t* getMotorstats();
	uint8_t* getBatterystats();
	uint8_t* getRangestats();
	void setKill(bool kill_status);
	
	bool readMessage();//(AvailableCallback fAvailable, ReadCallback fRead); //Includes CRC Checksum
	bool sendMessage(byte body[]);// (byte body[], WriteCallback fWrite, FlushCallback fflush);

	DisplayState getDisplayState();

private:
	void sendEmptyReply();
	void EncodeMessage(byte data[]);
	bool decodeMessage();
	
	void decodeDisplay();
	void decodeDisplayState();

	int16_t mapSpeed(int speed);

	int8_t crc8(int8_t crc, int8_t crc_data);
};

#endif