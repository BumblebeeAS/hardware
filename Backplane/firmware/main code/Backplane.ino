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
////Sensor & Actuator Board for BBAUV 3.0
//Firmware Version :             v1.0
//
// Written by Vanessa Cassandra
//
//###################################################
//###################################################
//###################################################

#include backplane.h
#include define.h
#include can_defines.h

//Constant declaration
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include <Adafruit_ADS1015.h>

MCP_CAN CAN(CAN_Chip_Select); 

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message

//static means the variable is only accessible within this file
static uint8_t smartkill_buf;
static uint8_t CAN_State_Buf[3]; //CAN tx & error statues buffer
static uint8_t StatsBuf[3]; //size to be determined
static uint8_t StatsBuf2[3]; //size to be determined

static uint32_t loopTime;
static uint32_t currentTime;

void setup(){
	Serial.begin(115200);
	//Enable all daughter boards and POE
	CAN_init();
	Serial.println("CAN OK");
	smartkill_init();
	Serial.println("Smart kill OK");
	Serial.println("All OK");
	currentTime = loopTime = millis();


}

void loop(){
	//send and receive messages every xx ms
	currentTime = millis();
	if(currentTime > loopTime){
		checkCANmsg();
		checkSmartKill();
		Serial.println("Thruster: %d", smartkill_buf[0]);
		Serial.println("SA: %d", smartkill_buf[1]);
		Serial.println("Telemetry: %d", smartkill_buf[2]);

		StatsBuf[0] = currentReading1;
		StatsBuf[1] = currentReading2;
		StatsBuf[2] = voltageReading5V;
		StatsBuf[3] = voltageReading12V;
		StatsBuf2[0] = dummy;


		//Publish stats onto the CAN bus
		CAN.sendMsgBuf(CAN_backplane_stats, 0, 3, StatsBuf);
		Serial.println(StatsBuf[0]);
		Serial.println(StatsBuf[1]);
		Serial.println(StatsBuf[2]);

		//check CAN status and send back status
		CAN_State_Buf[0]=CAN.checkError();
		CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
		CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
		//Serial.println(CAN_State_Buf[0]);
		//Serial.println(CAN_State_Buf[1]);
		//Serial.println(CAN_State_Buf[2]);
		CAN.sendMsgBuf(CAN_backplane_BUS_stats, 0, 3, CAN_State_Buf);

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
	pinMode(TELE_EN, OUTPUT);
	pinMode(POE_NAV, OUTPUT);
	pinMode(POE_SONAR, OUTPUT);
	pinMode(POE_ACOU, OUTPUT);
	//enable everything
	digitalWrite(SA_EN, HIGH);
	digitalWrite(THRUST_EN, HIGH);
	digitalWrite(TELE_EN, HIGH);
	digitalWrite(POE_NAV, HIGH);
	digitalWrite(POE_SONAR, HIGH);
	digitalWrite(POE_ACOU, HIGH);
}

void checkSmartKill(){
	if(smartkill_buf[0] = TRUE)	//bit 0 = thruster
		digitalWrite(THRUST_EN, LOW);	//kill thruster
	else
		digitalWrite(THRUST_EN, HIGH);	//enable thruster
	

	if(smartkill_buf[1] = TRUE)	//bit 1 = SA
		digitalWrite(SA_EN, LOW);	//kill SA
	else
		digitalWrite(SA_EN, HIGH);	//enable SA


	if(smartkill_buf[2] = TRUE)	//bit 2 = telemetery
		digitalWrite(TELE_EN, LOW);	//kill telemetery
	else
		digitalWrite(TELE_EN, HIGH);	//enable telemetery
	
}


void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		if(CAN.getCanId() == CAN_backplane_kill){	//ID = 11
			//put into smartkill buffer
			smartkill_buf = buf[0];
		}else{
			//throw an error
		}

		CAN.clearMsg();
	}
}