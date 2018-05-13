#include <can_defines.h>
#include "can_auv_define.h"
#include "define.h"
#include <can.h>
#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include "Thrusters.h"

void CAN_init();
void checkCANmsg();

static uint32_t loopTime;
static uint32_t currentTime;
static uint32_t can_bus_loop;
uint8_t thruster_bus_stats[8];
static uint32_t Thruster_loop_20; //20ms loop for thrusters
static uint8_t hb_sync_ctr = 0;
static uint8_t thruster_enable = 0;
static uint32_t hb_sync_timer = 0;
static uint32_t hb_local_timer = 0;
uint16_t heartbeat_ctr = 0;

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

// ESC	  - THRUSTER
// RED	  - RED
// YELLOW - WHITE
// BLUE	  - GREEN

Servo servo1, servo2, servo3, servo4, servo5, servo6, servo7, servo8;
Thrusters thruster1(1, servo1, THRUSTER_1, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster2(2, servo2, THRUSTER_2, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster3(3, servo3, THRUSTER_3, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster4(4, servo4, THRUSTER_4, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster5(5, servo5, THRUSTER_5, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster6(6, servo6, THRUSTER_6, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster7(7, servo7, THRUSTER_7, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);
Thrusters thruster8(8, servo8, THRUSTER_8, FORWARD_MAX, FORWARD_MIN, REVERSE_MIN, REVERSE_MAX);

unsigned short Thruster_buf[8] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500}; //buffer for Thrusters

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message

uint32_t test_time = 0;
uint32_t heartbeat_loop = 0;
uint8_t hb_buf[2];

#define _MANUAL_RUN_
#ifdef _MANUAL_RUN_
int motorspeed;
char inputstr[10] = { '\n' };
int serialidx = 0;
#endif


void setup(){
	Serial.begin(115200);
	Serial.println("Hi, I'm Thruster Board!");
	Serial1.begin(19200);
	CAN_init(); //initialise CAN
	can_bus_loop = heartbeat_loop = millis();
	
	thruster1.init();
	thruster2.init();
	thruster3.init();
	thruster4.init();
	thruster5.init();
	thruster6.init();
	thruster7.init();
	thruster8.init();
}

void loop()
{
	/*****************************************/
	/*  Heartbeat							 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		CAN.setupCANFrame(hb_buf, 0, 1, HEARTBEAT_TB);
		CAN.sendMsgBuf(CAN_heartbeat, 0, 1, hb_buf);
		hb_local_timer = millis();
		if(0)
		//if (hb_local_timer - hb_sync_timer > 3000)
		{
			//no SBC heartbeat
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
	
	/*
	if ((millis() - can_bus_loop) > 1000)
	{
	CAN.setupCANFrame(thruster_bus_stats, 0, 1, CAN.checkError());
	CAN.setupCANFrame(thruster_bus_stats, 1, 1, CAN.checkTXStatus(0));
	CAN.setupCANFrame(thruster_bus_stats, 2, 1, CAN.checkTXStatus(1));
	CAN.sendMsgBuf(CAN_thruster_BUS_stats, 0, 4, thruster_bus_stats);
	can_bus_loop = millis();
	}
	*/
	
#ifdef _MANUAL_RUN_
	// To manually run thrusters if SBC not up
	if (Serial.available())
	{
		byte input;
		input = Serial.read();
		inputstr[serialidx] = input;

		/*
		for (int i = 0; i <= serialidx; i++)
		{
		Serial.print(inputstr[i]);
		}
		Serial.println();
		*/
		if (input == '\n' || input == '\r')
		{
			inputstr[serialidx] = '\0';
			if (inputstr[0] == 'u')
			{
				Serial.println("+++");
				motorspeed = motorspeed + 100;
				Serial.println(motorspeed);
			}
			else if (inputstr[0] == 'd')
			{
				Serial.println("---");
				motorspeed = motorspeed - 100;
				Serial.println(motorspeed);
			}
			else if (isalpha(inputstr[0]))
			{
				motorspeed = 0;
				Serial.println("STOP");
			}
			else
			{
				motorspeed = atoi(inputstr);
				Serial.println(motorspeed);
			}
			serialidx = -1;
		}
		serialidx++;
	}
#endif

	currentTime = millis();
	checkCANmsg();
	if (currentTime > Thruster_loop_20 + 20){
		
		if(!thruster_enable){
			// stop thrusters
			Thruster_buf[0] = 0;
			Thruster_buf[1] = 0;
			Thruster_buf[2] = 0;
			Thruster_buf[3] = 0;
			Thruster_buf[4] = 0;
			Thruster_buf[5] = 0;
			Thruster_buf[6] = 0;
			Thruster_buf[7] = 0;
		}
		for(int i = 0; i < 8; i++)
		{
			Serial.print(i);
			Serial.print(": ");
			Serial.print(Thruster_buf[i]);
			Serial.print("\t");
		}
		Serial.println("run");
		
#ifdef _MANUAL_RUN_
		thruster1.mov(motorspeed);
		thruster2.mov(motorspeed);
		thruster3.mov(motorspeed);
		thruster4.mov(motorspeed);
		thruster5.mov(motorspeed);
		thruster6.mov(motorspeed);
		thruster7.mov(motorspeed);
		thruster8.mov(motorspeed);
#else
		thruster1.mov(Thruster_buf[0]);
		thruster2.mov(Thruster_buf[1]);
		thruster3.mov(Thruster_buf[2]);
		thruster4.mov(Thruster_buf[3]);
		thruster5.mov(Thruster_buf[4]);
		thruster6.mov(Thruster_buf[5]);
		thruster7.mov(Thruster_buf[6]);
		thruster8.mov(Thruster_buf[7]);
#endif
		
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
	CAN.init_Filt(3, 0, CAN_ST_stats);	//Heartbeat Message
	
}

void checkCANmsg()
{
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		//FLAGMsg = false;
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		//Serial.println(CAN.getCanId());
		
		switch (CAN.getCanId()){
			case CAN_thruster_1:{
				Serial.print("thruster ");
				for (int i = 0; i < 4; i++){
					Serial.print(i);
					Serial.print(": ");
					Thruster_buf[i] = map(CAN.parseCANFrame(buf, i * 2, 2), 0, 6400, 1000, 2000);
					Serial.print(Thruster_buf[i]);
					Serial.print("\t");
				}
				Serial.println("");
				break;
			}
			
			case CAN_thruster_2:{
				Serial.println("thruster ");
				for (int i = 0; i < 4; i++)
				{
					Serial.print(i+4);
					Serial.print(": ");
					Thruster_buf[i + 4] = map(CAN.parseCANFrame(buf, i * 2, 2), 0, 6400, 1000, 2000);
					Serial.print(Thruster_buf[i + 4]);
					Serial.print("\t");
				}
				Serial.println("");
				break;
			}
			
			case CAN_heartbeat:			
			if(CAN.parseCANFrame(buf, 0, 1) == HEARTBEAT_SBC){
				Serial.println("heartbeat");
				hb_sync_timer = millis();
			}
			break;
			
			case CAN_ST_stats:
			if(CAN.parseCANFrame(buf, 3, 1) == 1){
				Serial.println("Leaking");
				thruster1.mov(THROTTLE_STOP);
				thruster2.mov(THROTTLE_STOP);
				thruster3.mov(THROTTLE_STOP);
				thruster4.mov(THROTTLE_STOP);
				thruster5.mov(THROTTLE_STOP);
				thruster6.mov(THROTTLE_STOP);
				thruster7.mov(THROTTLE_STOP);
				thruster8.mov(THROTTLE_STOP);
				while(1);
			}			
			
			default:
			//TODO=>throw an error
			break;
		}
		
		CAN.clearMsg();
	}
}
