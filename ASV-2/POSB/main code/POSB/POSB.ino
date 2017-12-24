//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        

// Written by Ng Ren Zhi

// POSB firmware:
//	Thruster control (to ESC via CAN)
//	Battery monitoring (via RS485)
//	Humidity & Temperature
//	Light Tower

// Change Log for v1.0:
// - Initial commit

//###################################################
//###################################################

#include <can_defines.h>
#include <can.h>
#include <HIH613x.h>
#include <Xbee.h>

#include "can_asv_defines.h"
#include "defines.h"
#include "Arduino.h"
#include "Roboteq.h"

MCP_CAN CAN(8);
uint8_t buf[8];
uint8_t len = 0;

Roboteq roboteq1(&CAN, 1);
Roboteq roboteq2(&CAN, 2);
int16_t speed1 = 0;
int16_t speed2 = 0;
int16_t speed3 = 0;
int16_t speed4 = 0;

uint8_t posb_stat_buf[2] = {128,256};
uint8_t esc1_stat_buf[7] = {0, 0, 0, 0x0, 0x14, 0x1, 0x2C}; // 0, 0, 0, 200, 300
uint8_t esc2_stat_buf[7] = {0, 0, 0, 0x1, 0xF4, 0x1, 0x90}; // 0, 0, 0, 500, 400
uint8_t batt1_stat_buf[6] = {25, 0x2, 0x58, 0x1, 0x2C, 75}; // 25, 600, 300, 75
uint8_t batt2_stat_buf[6] = {25, 0x2, 0xBC, 0x1, 0x2C, 50}; // 25, 700, 300, 50
uint32_t heartbeat_loop;
uint32_t batt_loop;
uint32_t windspeed_loop;
uint8_t batt_ctr = 0;

bool heartbeat_esc1 = false;
bool heartbeat_esc2 = false;
bool heartbeat_batt1 = false;
bool heartbeat_batt2 = false;

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop = 0; //250ms loop Publish temp and humidity
uint8_t humid_ctr = 0;

void setup()
{	
	// CAN BUS INIT
	Serial.begin(115200);
	CAN_init();

	// ESC INIT
	roboteq1.init();
	roboteq2.init();

	Temp_Humid_loop = millis();
	heartbeat_loop = millis();
	batt_loop = millis();
	windspeed_loop = millis();
}

void loop()
{

	/************************************/
	/*			Thruster Control		*/
	/************************************/

	/************************************/
	/*			Battery Monitoring		*/
	/************************************/

	/************************************/
	/*	Humidity & Temperature Sensor	*/
	/************************************/

	readTempHumid();

	/************************************/
	/*			Light Tower				*/
	/************************************/

	// Heartbeat
	if((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		publishCAN_heartbeat();
		heartbeat_esc1 ^= 1;
		heartbeat_batt1 ^= 1;
		heartbeat_loop = millis();
	}
	// Windspeed	
	if((millis() - windspeed_loop) > WINDSPEED_TIMEOUT)
	{
		publishCAN_windspeed();
		windspeed_loop = millis();
	}
	// Batt + ESC
	if ((millis() - batt_loop) > BATT_TIMEOUT)
	{
		switch (batt_ctr)
		{
		case 0:
			publishCAN_esc1_stats();
			break;
		case 1:
			publishCAN_esc2_stats();
			break;
		case 2:
			publishCAN_batt1_stats();
			break;
		case 3:
			publishCAN_batt2_stats();
			batt_ctr = -1;
			break;
		}
		batt_ctr++;
		batt_loop = millis();
	}

	// CAN RX
	checkCANmsg();
}

//======== TEMP HUMID SENSOR =============//

void readTempHumid()
{
	if (millis() - Temp_Humid_loop > HUMID_TIMEOUT)
	{
		switch (humid_ctr)
		{
		case 0:
#ifdef _TEMP_
			humidTempSensor.measurementRequest();
#endif
			humid_ctr++;
			Temp_Humid_loop = millis();
			break;
		case 1:
			//Get I2C Data
			//push into send state buf
#ifdef _TEMP_
			humidTempSensor.dataFetch();
			posb_stat_buf[0] = humidTempSensor.getTemperature() + 0.5;
			posb_stat_buf[1] = humidTempSensor.getHumidity() + 0.5;
#endif
			publishCAN_posbstats();
			humid_ctr = 0;
			Temp_Humid_loop = millis();
			break;
		default:
			break;
		}
	}
}

//============= CAN =============//

void CAN_init()
{
START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
	{
		Serial.println("CAN BUS: OK");
	}
	else
	{
		Serial.println("CAN BUS: FAILED");
		Serial.println("CAN BUS: Reinitializing");
		delay(1000);
		goto START_INIT;
	}
	Serial.println("INITIATING TRANSMISSION...");
}

void publishCAN_posbstats()
{
	CAN.sendMsgBuf(CAN_POPB_stats,0,2,posb_stat_buf);
}
void publishCAN_heartbeat()
{
	buf[0] = HEARTBEAT_POSB;
	buf[1] = heartbeat_esc1 + (heartbeat_esc2 >> 1) + (heartbeat_batt1 >> 2) + (heartbeat_batt2 >> 3);
	CAN.sendMsgBuf(CAN_heartbeat,0,2,buf);
}
void publishCAN_windspeed()
{
	CAN.setupCANFrame(buf,0,2,1234);
	CAN.setupCANFrame(buf,2,2,2345);
	CAN.sendMsgBuf(CAN_wind_speed,0,4,buf);
}
void publishCAN_esc1_stats()
{
	CAN.sendMsgBuf(CAN_esc1_motor_stats, 0, 7, esc1_stat_buf);
}
void publishCAN_esc2_stats()
{
	CAN.sendMsgBuf(CAN_esc2_motor_stats, 0, 7, esc2_stat_buf);
}
void publishCAN_batt1_stats()
{
	CAN.sendMsgBuf(CAN_battery1_motor_stats, 0, 6, batt1_stat_buf);
}
void publishCAN_batt2_stats()
{
	CAN.sendMsgBuf(CAN_battery2_motor_stats, 0, 6, batt2_stat_buf);
}
void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		switch(CAN.getCanId()){
		case CAN_thruster:
			speed1 = CAN.parseCANFrame(buf,0,2) - 3200;
			speed2 = CAN.parseCANFrame(buf,2,2) - 3200;
			speed3 = CAN.parseCANFrame(buf,4,2) - 3200;
			speed4 = CAN.parseCANFrame(buf,6,2) - 3200;
			Serial.print(" 1: ");
			Serial.print(speed1);
			Serial.print(" 2: ");
			Serial.print(speed2);
			Serial.print(" 3: ");
			Serial.print(speed3);
			Serial.print(" 4: ");
			Serial.println(speed4);
			break;
		default:
			Serial.println("Others");
			break;
		}
		Serial.print("ID: ");
		Serial.print(CAN.getCanId());
		Serial.print(" Len: ");
		Serial.print(len);
		Serial.print(" Payload: ");
		for(int i = 0; i < len; i++)
			Serial.print(CAN.parseCANFrame(buf,i,1),HEX);
		Serial.println();

		CAN.clearMsg();
	}
}