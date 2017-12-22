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

MCP_CAN CAN(8);
uint8_t buf[8];

uint8_t posb_stat_buf[2];

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop = 0; //250ms loop Publish temp and humidity
uint8_t humid_ctr = 0;

void setup()
{	
	// CAN BUS INIT
	Serial.begin(115200);
	CAN_init();
	
	Temp_Humid_loop = millis();
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
}

//======== TEMP HUMID SENSOR =============//

void readTempHumid()
{
	if (millis() - Temp_Humid_loop > HUMID_TIMEOUT)
	{
		switch (humid_ctr)
		{
		case 0:
			humidTempSensor.measurementRequest();
			humid_ctr++;
			Temp_Humid_loop = millis();
			break;
		case 1:
			//Get I2C Data
			//push into send state buf
			humidTempSensor.dataFetch();
			posb_stat_buf[0] = humidTempSensor.getTemperature() + 0.5;
			posb_stat_buf[1] = humidTempSensor.getHumidity() + 0.5;
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