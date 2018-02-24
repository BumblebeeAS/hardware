#pragma once
#ifndef _TORQEEDO_
#define _TORQEEDO_

#include "Arduino.h"

#define MAX_PACKET_SIZE 40 //in bytes
#define END_MARKER 0xF6

#define TORQEEDO1_RXEN 22
#define TORQEEDO1_DXEN 24
#define TORQEEDO2_RXEN 26
#define TORQEEDO2_DXEN 28
#define TORQEEDO1_ON 12
#define TORQEEDO2_ON 12

#define BATTERY_ON_DURATION 600	// At least 0.5 sec
#define BATTERY_OFF_DURATION 5000	// At least 3 sec

#define POWERSEQ_OFF 0

#define BATT_RESET_COUNT 30 // number of request before re-trigger start up.

#define BAUDRATE 19200

/*=======================================================
				PACKET DELIMITERS
	-------------------------------------------------------*/
#define PACKET_START 0xAC /* marks every packet start */
#define PACKET_END 0xAD /* marks every packet end */
#define PACKET_ESCAPE 0xAE /* marks escaped bytes in packet body */
#define PACKET_ESCAPE_MASK 0x80 /* xor with escaped bytes in packet body */

/*=======================================================
				DEVICE ADDRESSES
	-------------------------------------------------------*/
// Only valid for master
//Address when slave replies is 0x00
#define DeviceId_Master	0x00
#define DeviceId_Remote1	0x14
#define DeviceId_Display	0x20
#define DeviceId_Motor	0x30
#define DeviceId_Battery	0x80

/*=======================================================
				MESSAGE ID
-------------------------------------------------------*/

/** BATTERY**/
#define BATTERY_STATUS_ID	0x04
#define BATTERY_TEMPS_ID	0x20
#define BATTERY_VOLTS_ID	0x21

/** BATT_MSG TYPE **/
#define MSG_STATUS 0
#define MSG_TEMP 1
#define MSG_VOLT 2

void write1(byte content);
byte read1();
int available1();
void flush1();
void write2(byte content);
byte read2();
int available2();
void flush2();

typedef struct {
	uint16_t statusFlag=0;
	uint16_t warningFlag=0;
	uint16_t errorFlag=0;
	int16_t cellMaxTemp; // 0.1 dC
	int16_t pcbMaxTemp; // 0.1 dC
	uint16_t voltage; // 0.1 V
	int16_t battCurrent; // 0.01 A
	uint16_t powerDrawn; // 1 W
	uint16_t maxLearnedCap; // 0.01 Ah
	uint16_t capRemaining; // 0.01 Ah
	uint16_t capPercent; // 1 %
	uint16_t timeLeft; // minutes
	uint16_t waterSense; // wet == 0
	int16_t tempCell1;
	int16_t tempCell2;
	int16_t tempPCB1;
	int16_t tempPCB2;
	uint16_t voltCell1;
	uint16_t voltCell2;
	uint16_t voltCell3;
	uint16_t voltCell4;
	uint16_t voltCell5;
	uint16_t voltCell6;
	uint16_t voltCell7;

} BatteryState;

class Torqeedo
{
private:
	uint8_t RX_ENABLE;
	uint8_t DX_ENABLE;
	uint8_t ON_PIN;
	uint8_t _battNum;

	bool msgStart  = false;
	uint8_t len = 0; //length of packet body including header
	uint8_t _checksum;
	uint8_t msg_type = 0;

	uint16_t onPinPeriod = 0;
	uint32_t onTime = 0;
	
	typedef void(*WriteCallback)  (const byte what);    // send a byte to serial port
	typedef int(*AvailableCallback)  ();    // return number of bytes available
	typedef byte(*ReadCallback)  ();    // read a byte from serial port
	typedef void(*FlushCallback)  ();    // read a byte from serial port

	WriteCallback _write;
	AvailableCallback _available;
	ReadCallback _read;
	FlushCallback _flush;

public:
	int requestCount = 0;
	static int powerSeq;
	bool kill = false;
	byte data[MAX_PACKET_SIZE];
	BatteryState battData;

	Torqeedo(int RXEN, int DXEN, int ON, int batt_num);
	~Torqeedo();
	void init();
	
	void onBattery(bool on_status);
	bool checkBatteryOnOff();
	void checkBatteryConnected();
	void requestUpdate();

	void getData();
	void resetData();
	uint16_t getVoltage();
	int16_t getCurrent();
	uint16_t getCapacity();
	int16_t getTemperature();

	bool readMessage();//(AvailableCallback fAvailable, ReadCallback fRead); //Includes CRC Checksum
	bool sendMessage(byte body[]);// (byte body[], WriteCallback fWrite, FlushCallback fflush);

private:
	void encodeMessage(byte data[]);
	bool decodeMessage();

	int8_t crc8(int8_t crc, int8_t crc_data);
};

#endif
