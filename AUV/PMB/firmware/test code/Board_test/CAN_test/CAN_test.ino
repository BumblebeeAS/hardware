#include <SPI.h> // for CAN Controller
#include "can.h"
#include "can_defines.h"

//dummy values to test CAN
uint8_t PMB_stats1[8] = {0, 1, 2, 3 , 4, 5, 6, 7};
uint8_t PMB_stats2[8] = {8, 9, 10, 11, 12, 13, 14, 15};
uint8_t PMB_stats3[5] = {16, 17, 18};

uint8_t PMB_CAN_stats[3];

double last_run = 0;
bool sendCanStats = true;
bool FLAGMsg = false;
MCP_CAN CAN(8);               //Set Chip Select to pin 8

void MCP2515_ISR(){
  FLAGMsg = true;      
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

    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
    
    //register number, extension, 
	// CAN.init_Filt(0, 0, CAN_pressure);//Pressure
	// CAN.init_Filt(1, 0, CAN_manipulator);//Manipulator
	// CAN.init_Filt(2, 0, CAN_LED);//LED Array
	// CAN.init_Filt(3, 0, CAN_SA_stats);//Sensor & Actuator Statistics 
	// CAN.init_Filt(4, 0, CAN_SA_BUS_stats);//Sensor & Actuator CAN Stats

    //Interrupt not working yet 0 for Pin D2, Interrupt service routine ,Falling edge 

}

void setup(){
	CAN_init();
	Serial.begin(9600);
}

void loop(){
	int now = millis();
	if (now - last_run > 500){
    last_run = now;
		CAN.sendMsgBuf(CAN_PMB1_stats, 0, 8, PMB_stats1);
		CAN.sendMsgBuf(CAN_PMB1_stats2, 0, 8, PMB_stats2);
		CAN.sendMsgBuf(CAN_PMB1_stats3, 0, 5, PMB_stats3);

		sendCanStats != sendCanStats;
		if(sendCanStats){
		 //    Serial.println(StatuesBuf[0]);
			// Serial.println(StatuesBuf[1]);
			// Serial.println(StatuesBuf[2]);
			PMB_CAN_stats[0]=CAN.checkError();
			PMB_CAN_stats[1]=CAN.checkTXStatus(0);//check buffer 0
			PMB_CAN_stats[2]=CAN.checkTXStatus(1);//check buffer 1
			//Serial.println(CAN_State_Buf[0]);
			//Serial.println(CAN_State_Buf[1]);
			//Serial.println(CAN_State_Buf[2]);
			CAN.sendMsgBuf(CAN_PMB1_BUS_stats, 0, 3, PMB_CAN_stats);
		}
	}
}
