//###################################################
//###################################################
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
////Backplane for BBAUV 3.0
//Firmware Version :             v1.0
//
// Written by Vanessa Cassandra
//
//###################################################
//###################################################
//###################################################

#include "backplane.h"
#include "define.h"
#include "can_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include <Adafruit_ADS1015.h>
#include <TempAD7414.h>
#include <Wire.h>

TempAD7414 temp_sens(96);

MCP_CAN CAN(CAN_Chip_Select); 

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message

//static means the variable is only accessible within this file
static uint8_t smartkill_buf;
static uint8_t CAN_State_Buf[3]; //CAN tx & error statues buffer
static uint8_t StatsBuf[3]; //size to be determined
static uint8_t StatsBuf2[3]; //size to be determined
static uint8_t heartbeat[1];
static uint8_t currentReading[3];
static uint8_t voltageReading5V;
static uint8_t voltageReading12V;
static uint8_t dummy;
static uint8_t temp;

static uint32_t smartkillTime;
static uint32_t statsTime;
static uint32_t canStatsTime;
static uint32_t currentTime;

void setup(){
	Serial.begin(115200);
	//Enable all daughter boards and POE
	CAN_init();
	Serial.println("CAN OK");
	smartkill_init();
	Serial.println("Smart kill OK");
	Serial.println("All OK");
	smartkillTime = statsTime = canStatsTime = millis();

  temp_sens.initTempAD7414(); //temp sensor
  heartbeat[0] = HEARTBEAT_BACKPLANE;

}

void loop(){
	//send and receive messages every xx ms
	if((millis() - smartkillTime) > 250){
		checkCANmsg();
		checkSmartKill();
    Serial.println("SMARTKILL:");
		Serial.println(smartkill_buf, BIN);

    smartkillTime = millis();
	}

  if((millis() - statsTime) > 500){
    temp = temp_sens.getTemp();
    Serial.println(temp);

    StatsBuf[0] = temp;  
//    Serial.println(StatsBuf[0]);
//    Serial.println(StatsBuf[1]);
    CAN.sendMsgBuf(CAN_backplane_stats, 0, 1, StatsBuf); //id, extended frame(1) or normal(0), no of bytes sent, data
    CAN.sendMsgBuf(CAN_heartbeat, 0, 1, heartbeat);
    statsTime = millis();
  }
 
//		StatsBuf[0] = currentReading[0];
//		StatsBuf[1] = currentReading[1];
//		StatsBuf[2] = voltageReading5V;
//		StatsBuf[3] = voltageReading12V;
//		StatsBuf2[0] = dummy;

  if((millis() - canStatsTime) > 1000){
		//check CAN status and send back status
		CAN_State_Buf[0]=CAN.checkError();
		CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
		CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
    Serial.println("CAN STATUS:");
		Serial.println(CAN_State_Buf[0]);
		Serial.println(CAN_State_Buf[1]);
		Serial.println(CAN_State_Buf[2]);
		CAN.sendMsgBuf(CAN_backplane_BUS_stats, 0, 3, CAN_State_Buf); //CAN_backplane_BUS_stats ID = 22

    canStatsTime = millis();

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

    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
    
    //register number, extension, 
	CAN.init_Filt(0, 0, CAN_backplane_kill);	//smart kill
	CAN.init_Filt(1, 0, CAN_backplane_stats);	//Backplane statistics
	CAN.init_Filt(2, 0, CAN_backplane_stats2);	//Backplane statistics 2
	CAN.init_Filt(3, 0, CAN_backplane_BUS_stats);	//Backplane BUS stats

}

void smartkill_init(){
	pinMode(SA_EN, OUTPUT);
	pinMode(THRUST_EN, OUTPUT);
	DDRD = DDRD | B01000000; //set PD6 as output (TELE_EN)
	pinMode(POE_NAV, OUTPUT);
	pinMode(POE_SONAR, OUTPUT);
	pinMode(POE_ACOU, OUTPUT);
  
	//enable everything
	digitalWrite(SA_EN, HIGH);
	digitalWrite(THRUST_EN, HIGH);
	PORTD |= B01000000; //enable telemetry
	digitalWrite(POE_NAV, HIGH);
	digitalWrite(POE_SONAR, HIGH);
	digitalWrite(POE_ACOU, HIGH);
}

void checkSmartKill(){
	if((smartkill_buf & 0x01) == 1)	//bit 0 = thruster
		PORTF &= ~(B00000001);	//kill thruster
	else
		PORTF |= B00000001;	//enable thruster
	

	if(((smartkill_buf >> 1) & 0x01) == 1)	//bit 1 = SA
		digitalWrite(SA_EN, LOW);	//kill SA
	else
    digitalWrite(SA_EN, HIGH);  //enable SA
		
	if(((smartkill_buf >> 2) & 0x01) == 1)	//bit 2 = telemetery
		PORTD &= ~(B01000000);	//kill telemetery
	else
		PORTD |= B01000000;	//enable telemetery
	
}


void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		if(CAN.getCanId() == CAN_backplane_kill){	//ID = 11
			//put into smartkill buffer
      Serial.println("RECEIVED!");
			smartkill_buf = buf[0];
		}else{
			//throw an error
		}

		CAN.clearMsg();
	}
}
