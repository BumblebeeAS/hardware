#pragma once
#ifndef _TORQEEDO_
#define _TORQEEDO_

#include"Arduino.h"

#define MAX_PACKET_SIZE 40 //in bytes
#define END_MARKER 0xF6

#define TORQEEDO1_RXEN 2
#define TORQEEDO1_DXEN 3
#define TORQEEDO2_RXEN 4
#define TORQEEDO2_DXEN 5

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
	void write2(byte content);
	byte read2();
	int available2();
	
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
	bool kill = false;
	int16_t hardcodespeed = 0;
	int startUpCount = 0;
	bool speedZero = true;
	
	typedef void(*WriteCallback)  (const byte what);    // send a byte to serial port
	typedef int(*AvailableCallback)  ();    // return number of bytes available
	typedef byte(*ReadCallback)  ();    // read a byte from serial port

public:
	Torqeedo(int RXEN, int DXEN, int thruster_num);
	~Torqeedo();
	void init();

	bool setMotorDrive(int speed);
	uint8_t* getMotorstats();
	uint8_t* getBatterystats();
	uint8_t* getRangestats();
	
	bool readMessage(AvailableCallback fAvailable, ReadCallback fRead); //Includes CRC Checksum
	bool sendMessage(byte body[], WriteCallback fWrite);

	DisplayState getDisplayState();

private:
	//bool readPlainMessage(AvailableCallback fAvailable, ReadCallback fRead, char message[maxMsgLen+3+1]);
	void sendEmptyReply();
	void EncodeMessage(byte data[]);
	bool decodeMessage();
	
	void decodeDisplay();
	void decodeDisplayState();

	int8_t crc8(int8_t crc, int8_t crc_data);
};

#endif