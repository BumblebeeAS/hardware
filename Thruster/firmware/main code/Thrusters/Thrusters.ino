#include "can_defines.h"
#include "define.h"
#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>
#include <can.h>
#include <can_defines.h>
#include <stdint.h>
#include <Thrusters.h>
#include <math.h>
#include <Brushed.h>

static uint32_t loopTime;
static uint32_t currentTime;

static uint32_t Thruster_loop_20;//20ms loop for thrusters

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

int16_t Thruster_buf[8];//buffer for Thrusters

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message

boolean FLAGMsg = false;

Brushed seabotix; //initialise all 6 seabotix thrusters
Thrusters videoray(Videoray_1, Videoray_2); //initialise both videoray thrusters

void setup(){
  seabotix.begin();
  CAN_init(); //initialise CAN
}

void loop()
{
	currentTime=millis(); 
	if(currentTime > Thruster_loop_20 + 20){
		checkCANmsg();
		for(int j = 0; j < 8; j++)
		{
			if (j<2)
			{
				videoray.mov(Thruster_buf[j]);
			}
			else
			{
				seabotix.run(j+1,Thruster_buf[j]);
			}
		}
        }
}

void CAN_init(){
START_INIT:
    if(CAN_OK == CAN.begin(CAN_1000KBPS)){                   // init can bus : baudrate = 500k
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

    CAN.init_Mask(0, 0, 0x3ff);	// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);	// you need to set both of them

    CAN.init_Filt(0, 0, CAN_thruster_1);	//1st Thruster Message
    CAN.init_Filt(1, 0, CAN_thruster_2);	//2nd Thruster Message


}

void checkCANmsg()
{
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		FLAGMsg = false;
		Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		switch (CAN.getCanId()){

		case CAN_thruster_1:{ 
			for(int i = 0;i<4;i++)	Thruster_buf[i] = CAN.parseCANFrame(buf, i*2, 2) - 3200;
			break;
		}
		case CAN_thruster_2:{
			for(int i = 0;i<4;i++)	Thruster_buf[i + 4] = CAN.parseCANFrame(buf, i*2, 2) - 3200;
			break;
		}
		default:{
			//TODO=>throw an error 
		}

		}
		CAN.clearMsg();
	}
}
