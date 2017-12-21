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
//POSB for BBASV 2.0
//Firmware Version :             v1.0
//POSB Firmware for ASV 2.0
//		Telemetry LCD Display
//		Frisky Receiver (CPPM & DAC)
//		XBee
//
// Written by Ren Zhi
// Change log v0.0:
//
//###################################################
//###################################################
//###################################################

#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include "LCD_Driver.h"
//#include "Telemetry.h"
#include "define.h"
#include "can_defines.h"
#include "can_asv_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>

LCD screen = LCD(SCREEN_CS, SCREEN_RESET);  //screen
int test;
char string[200] = "Lorem ipsum dolor sit amet, consectetur adipiscing";
char u;


MCP_CAN CAN(CAN_Chip_Select); 

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message


//static means the variable is only accessible within this file
static uint8_t CAN_State_Buf[3]; //CAN tx & error statues buffer
static uint8_t internalStats[INT_STAT_COUNT];
static uint16_t powerStats[POWER_STAT_COUNT];
static uint32_t posb_timeout;
static uint32_t ocs_timeout;
static uint32_t rc_timeout;
static uint32_t sbc_timeout;
static uint32_t batt1_timeout;
static uint32_t batt2_timeout;
static uint32_t heartbeat_timeout[HB_COUNT];

static uint32_t loopTime;
static uint32_t logTime;
static uint32_t currentTime;
static uint32_t canStatsTime;

void setup(){
	Serial.begin(115200);
	Serial1.begin(9600); //xbee serial
	CAN_init();
	Serial.println("CAN OK");
	screen.screen_init();
	Serial.println("Screen OK");
	currentTime = loopTime = millis();

	screen_prepare();
}


int32_t test_time = 0;
void loop(){
	//******* LCD SCREEN **********/

	/*CHECK FOR CONNECTION*/
	update_posb_stats();
	update_ocs_stats();
	update_rc_stats();
	update_sbc_stats();
	update_batt1_stats();
	update_batt2_stats();

	if((millis() - loopTime) > 1000){
		screen_update();
		loopTime = millis();
		update_heartbeat();
	}

	//******* RC RX **********/

	//******* RC TX **********/

	//******* OCS RX **********/

	//******* OCS TX **********/

	//******* CAN RX **********/
	checkCANmsg();

	//******* CAN TX **********/

	//  if((millis() - canStatsTime) > 1000){
	//    test_time = millis();
	//    //check CAN status and send back status
	//    CAN_State_Buf[0]=CAN.checkError();
	//    CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
	//    CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
	//    CAN.sendMsgBuf(CAN_telemetry_BUS_stats, 0, 3, CAN_State_Buf); //id, extended frame(1) or normal(0), no of bytes sent, data
	//    Serial.print("CAN publish time:");
	//    Serial.println(millis() - test_time);
	//    canStatsTime = millis();
	//  }
}

/*
=====Display=====
Heartbeat: POSB, POKB, POPB, LARS, Tele, OCS, RC, SBC, Batt1, Batt2, ESC1, ESC2
Battery: Voltage, Capacity, Current x 2
Internal Pressure??
Humidity
Temp: SBC, POSB
RSSI: OCS, RC
=====ControlLink=====
All ESC stats
All batt stats
Control Mode: Auto/Manual/E-stop
Remote Kill batt
INS
GPS
*/

//========= Update Data ==========
void update_posb_stats()
{
	if (millis() - posb_timeout > HB_TIMEOUT){
		internalStats[INT_PRESS] = 255;
		internalStats[HUMIDITY] = 255;
		internalStats[POSB_TEMP] = 255;
		posb_timeout = millis();
	}
}
void update_ocs_stats()
{
	if (millis() - posb_timeout > HB_TIMEOUT){
		internalStats[RSSI_OCS] = 255;
		ocs_timeout = millis();
	}
}
void update_rc_stats()
{
	if (millis() - posb_timeout > HB_TIMEOUT){
		internalStats[RSSI_RC] = 255;
		rc_timeout = millis();
	}
}
void update_sbc_stats()
{
	if (millis() - sbc_timeout > HB_TIMEOUT){
		internalStats[CPU_TEMP] = 255;
		sbc_timeout = millis();
	}
}
void update_batt1_stats()
{
	if (millis() - sbc_timeout > HB_TIMEOUT){
		powerStats[BATT1_CAPACITY] = 255;
		powerStats[BATT1_CURRENT] = 255;
		powerStats[BATT1_VOLTAGE] = 255;
		batt1_timeout = millis();
	}
}
void update_batt2_stats()
{
	if (millis() - sbc_timeout > HB_TIMEOUT){
		powerStats[BATT2_CAPACITY] = 255;
		powerStats[BATT2_CURRENT] = 255;
		powerStats[BATT2_VOLTAGE] = 255;
		batt2_timeout = millis();
	}
}

//========= LCD ==========

void screen_prepare(){
	screen.set_cursor(0, 0);
	screen.write_string("Int press:");
	screen.write_string("Humidity:");
	screen.write_string("CPU temp:");
	screen.write_string("POSB temp:");
	screen.write_string("RSSI OCS:");
	screen.write_string("RSSI RC:");
	screen.write_string("SBC OK:");
	screen.write_string("POKB OK:");
	screen.write_string("POSB OK:");
	screen.write_string("POPB OK:");
	screen.write_string("LARS OK:");
	screen.write_string("OCS OK:");
	screen.write_string("RC OK:");

	screen.set_cursor(400, 0);
	screen.write_string("Batt1 capacity:");
	screen.write_string("Batt2 capacity:");
	screen.write_string("Batt1 current:");
	screen.write_string("Batt2 current:");
	screen.write_string("Batt1 voltage:");
	screen.write_string("Batt2 voltage:");
	screen.write_string("Batt1 OK:");
	screen.write_string("Batt2 OK:");
	screen.write_string("ESC1 OK:");
	screen.write_string("ESC2 OK:");
	screen.write_value_string("CC buy us pineapple tart");
}

void screen_update(){
	screen.set_cursor(150, 0);
	for(int i = 0; i < INT_STAT_COUNT; i++)
	{
		screen.write_value_int(internalStats[i]);
	}

	screen.set_cursor(645, 0);
	for(int i = 0; i < POWER_STAT_COUNT; i++)
	{
		screen.write_value_int(powerStats[i]);
	}
}

void update_heartbeat()
{
	int i;
	/* CHECK FOR HEARTBEAT */
	screen.set_cursor(150, 210);
	for (i = 0; i < 7; i++){
		if((millis() - heartbeat_timeout[i]) > HB_TIMEOUT){
			screen.write_value_string("NO");
		}else
			screen.write_value_string("YES");
	}
	screen.set_cursor(550, 210);
	for (; i < 11; i++){
		if((millis() - heartbeat_timeout[i]) > HB_TIMEOUT){
			screen.write_value_string("NO");
		}else
			screen.write_value_string("YES");
	}
}

//========= CAN ==========

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

/*
=====Display=====
103: RSSI (OCS, RC), Control Mode
104: Heartbeat: POSB, POKB, POPB, LARS, Tele, OCS, RC, SBC, Batt1, Batt2, ESC1, ESC2
110,111: Battery stats
112,113: Motor stats
114: Remote Kill batt
115: INS
116: GPS
117: Temp(CPU)
118: Humidity, Temp(POSB)
*/

void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		switch(CAN.getCanId()){
		case CAN_heartbeat:
			{
			uint32_t device = CAN.parseCANFrame(buf, 0, 1);
			Serial.print("heartbeat: ");
			heartbeat_timeout[device] = millis();
			break;
			}

		case CAN_battery1_motor_stats:
			Serial.println("Batt1 stats");
			powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
			powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
			powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 3, 2);
			batt1_timeout = millis();
			break;

		case CAN_battery2_motor_stats:
			Serial.println("Batt1 stats");
			powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
			powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
			powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 3, 2);
			batt2_timeout = millis();
			break;

		case CAN_cpu_temp:
			Serial.println("sbc stats");
			internalStats[CPU_TEMP] = CAN.parseCANFrame(buf, 0, 1);
			sbc_timeout = millis();
			break;

		case CAN_POSB_stats:
			Serial.println("sbc stats");
			//internalStats[INT_PRESS] = CAN.parseCANFrame(buf, 2, 1);
			internalStats[INT_PRESS] = 255;
			internalStats[HUMIDITY] = CAN.parseCANFrame(buf, 1, 1);
			internalStats[POSB_TEMP] = CAN.parseCANFrame(buf, 0, 1);
			posb_timeout = millis();
			break;

		default:
			Serial.println("Others");
			break;
		}

		CAN.clearMsg();
	}
}