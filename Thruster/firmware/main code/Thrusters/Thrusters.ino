#include "can_defines.h"
#include <Arduino.h>
#include <Servo.h>
#include <can.h>
#include <can_defines.h>
#include <stdint.h>
#include <Thrusters.h>
#include <Brushed.h>

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

void setup(){
  Brushed seabotix();
  Thrusters videoray(Videoray_1, Videoray_2);
  CAN_init();
}

void loop(){
    
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

		if (CAN.getCanId() == CAN_thruster_1) {
		

			break;
		}
		default:{
			//TODO=>throw an error 
		}

		}
		CAN.clearMsg();
	}
}