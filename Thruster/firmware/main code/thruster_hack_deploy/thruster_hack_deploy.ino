#include "can_defines.h"
#include "define.h"
#include "smcDriver_v2.h"
#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include "can.h"
#include "can_defines.h"
#include "can_defines_main.h"
#include <stdint.h>
#include "Thrusters.h"
#include <math.h>

static uint32_t loopTime;
static uint32_t currentTime;
static uint32_t can_bus_loop;
uint8_t thruster_bus_stats[8];
static uint32_t Thruster_loop_20;//20ms loop for thrusters
static uint8_t hb_sync_ctr = 0;
static uint8_t thruster_enable = 0;
static uint32_t hb_sync_timer = 0;
static uint32_t hb_local_timer = 0;
uint16_t heartbeat_ctr = 0;

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

smcDriver mDriver(&Serial1);

int16_t Thruster_buf[8];//buffer for Thrusters

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message

boolean FLAGMsg = false;

Thrusters videoray(Videoray_1, Videoray_2); //initialise both videoray thrusters
uint32_t test_time = 0;
uint32_t heartbeat_loop = 0;
uint8_t hb_buf[2];


void setup(){
	Serial.begin(115200);
	Serial1.begin(19200);
	CAN_init(); //initialise CAN
	can_bus_loop = heartbeat_loop =  millis();
	mDriver.init();
}

void loop()
{
	/*****************************************/
	/*  Heartbeat							 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		CAN.setupCANFrame(hb_buf, 0, 0, HEARTBEAT_THRUSTER);
		hb_buf[0] = HEARTBEAT_THRUSTER;
		CAN.sendMsgBuf(CAN_heartbeat, 0, 1, hb_buf);
		hb_local_timer = millis();
		if (hb_local_timer - hb_sync_timer > 3000)
		{
			Serial.print("dis:");
			Serial.print(hb_local_timer);
			Serial.print(" ");
			Serial.println(hb_sync_timer);
			Serial.println("disabled");
			thruster_enable = 0;
		}
		else
		{
			thruster_enable = 1;
			Serial.print("ena:");
			Serial.print(hb_local_timer);
			Serial.print(" ");
			Serial.println(hb_sync_timer);
			Serial.println("enabled");
		}
		heartbeat_loop = millis();
	}

	/*****************************************/
	/*  Transmit CAN Diagnostics			 */
	/*****************************************/
	if ((millis() - can_bus_loop) > 1000)
	{
		CAN.setupCANFrame(thruster_bus_stats, 0, 1, CAN.checkError());
		CAN.setupCANFrame(thruster_bus_stats, 1, 1, CAN.checkTXStatus(0));
		CAN.setupCANFrame(thruster_bus_stats, 2, 1, CAN.checkTXStatus(1));
		CAN.sendMsgBuf(CAN_thruster_BUS_stats, 0, 4, thruster_bus_stats);
		can_bus_loop = millis();
	}

	currentTime = millis();
	checkCANmsg();
	if (currentTime > Thruster_loop_20 + 20){
		if (thruster_enable)
		{
			videoray.mov(Thruster_buf[0], Thruster_buf[1]);
			mDriver.setMotorSpeed(THRUSTER_3, Thruster_buf[2]);
			mDriver.setMotorSpeed(THRUSTER_4, Thruster_buf[3]);
			mDriver.setMotorSpeed(THRUSTER_5, Thruster_buf[4]);
			mDriver.setMotorSpeed(THRUSTER_6, Thruster_buf[5]);
			mDriver.setMotorSpeed(THRUSTER_7, Thruster_buf[6]);
			mDriver.setMotorSpeed(THRUSTER_8, Thruster_buf[7]);
		}
		else
		{
			Thruster_buf[0] = 0;
			Thruster_buf[1] = 0;
			Thruster_buf[2] = 0;
			Thruster_buf[3] = 0;
			Thruster_buf[4] = 0;
			Thruster_buf[5] = 0;
			Thruster_buf[6] = 0;
			Thruster_buf[7] = 0;
			videoray.mov(0,0);
			mDriver.setMotorSpeed(THRUSTER_3, 0);
			mDriver.setMotorSpeed(THRUSTER_4, 0);
			mDriver.setMotorSpeed(THRUSTER_5, 0);
			mDriver.setMotorSpeed(THRUSTER_6, 0);
			mDriver.setMotorSpeed(THRUSTER_7, 0);
			mDriver.setMotorSpeed(THRUSTER_8, 0);
		}
		Thruster_loop_20 = millis();
	}

}

void CAN_init(){
START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS)){                   // init can bus : baudrate = 500k
#if DEBUG_MODE == NORMAL
		Serial.println("CAN init ok!");
#endif  
	}
	else{
#if DEBUG_MODE == NORMAL
		Serial.println("CAN init fail");
		Serial.println("Init CAN again");
		delay(1000);
#endif           
		goto START_INIT;
	}

	//CAN.init_Mask(0, 0, 0x3ff);	// there are 2 mask in mcp2515,
	//CAN.init_Mask(1, 0, 0x3ff);	// you need to set both of them

	CAN.init_Filt(0, 0, CAN_thruster_1);	//1st Thruster Message
	CAN.init_Filt(1, 0, CAN_thruster_2);	//2nd Thruster Message
	CAN.init_Filt(2, 0, CAN_heartbeat);	//Heartbeat Message
}

void checkCANmsg()
{
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		FLAGMsg = false;
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf				
		//Serial.println(CAN.getCanId());
		switch (CAN.getCanId()){
		case CAN_thruster_1:{

			for (int i = 0; i < 4; i++)
			{
				Serial.println("thruster 1!");
				Thruster_buf[i] = CAN.parseCANFrame(buf, i * 2, 2) - 3200;
			}
			break;
		}
		case CAN_thruster_2:{
			Serial.println("thruster 2!");
			for (int i = 0; i<4; i++)	Thruster_buf[i + 4] = CAN.parseCANFrame(buf, i * 2, 2) - 3200;
			break;
		}
		case CAN_heartbeat:
			CAN.readMsgBuf(&len, buf);
			if (buf[0] == 5)
			{
				Serial.println(buf[0]);
				Serial.println("heartbeat!");
				hb_sync_timer = millis();
			}
			break;
		default:{
			//TODO=>throw an error 
		};
				break;
		}
		CAN.clearMsg();
	}
}
