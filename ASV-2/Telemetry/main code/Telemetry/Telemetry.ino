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

#include <XBee.h>
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include "LCD_Driver.h"
#include "Frisky_CPPM.h"
//#include "Telemetry.h"
#include "define.h"
#include "can_defines.h"
#include "can_asv_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>

LCD screen = LCD(SCREEN_CS, SCREEN_RESET);  //screen
Frisky rc = Frisky(RC_INT);

MCP_CAN CAN(CAN_Chip_Select); 

uint8_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message


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

static uint32_t heartbeat_loop = 0;
static uint32_t thruster_loop = 0;

int control_mode = AUTONOMOUS;
int32_t dir_forward = 0;
int32_t dir_side = 0;
int32_t dir_yaw = 0;
int32_t speed1 = 0;
int32_t speed2 = 0;
int32_t speed3 = 0;
int32_t speed4 = 0;

bool manualOperationMode = false;
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
XBee xbee = XBee();
uint8_t *payload;
uint8_t cmd[] = { 'D','B' };
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

void setup(){
	Serial.begin(115200);

	// CAN INIT
	CAN_init();
	Serial.println("CAN OK");

	// LCD INIT
	screen.screen_init();
	Serial.println("Screen OK");
	screen_prepare();

	// FRISKY INIT
	rc.init();

	// XBEE INIT
	Serial2.begin(XBEE_BAUDRATE);
	xbee.setSerial(Serial2);

	currentTime = loopTime = millis();
}


int32_t test_time = 0;
void loop(){
	//******* LCD SCREEN **********/

	reset_stats();

	if((millis() - loopTime) > 1000){
		screen_update();
		loopTime = millis();
		update_heartbeat();
	}

	/*************************************************/
	/*					RC RX					     */
	/* Read Frisky CPPM for Manual Thruster override */
	/*************************************************/
	get_directions();
	get_controlmode();
	convert_thruster_values();

	Serial.print(" 1: ");
	Serial.print(speed1);
	Serial.print(" 2: ");
	Serial.print(speed2);
	Serial.print(" 3: ");
	Serial.print(speed3);
	Serial.print(" 4: ");
	Serial.println(speed4);

	/********************************************/
	/*					RC TX				    */
	/* Send ASV Batt value to Frisky Controller */
	/********************************************/

	/**********************************************/
	/*					OCS RX					  */
	/* Read OCS XBEE for Manual Thruster override */
	/**********************************************/

	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			xbee.getResponse().getZBRxResponse(rx);
			payload = rx.getData();
			switch (payload[2])
			{
			case CAN_control_link:				
				if (payload[3]) manualOperationMode = true;
				else manualOperationMode = false;
			case CAN_manual_thruster:
				/*
				if(manualOperationMode)//Manual mode activated, overrides CAN data
				{
				speed1 = int16_t(CAN.parseCANFrame(payload, 4, 2)) - 1000;
				speed2 = int16_t(CAN.parseCANFrame(payload, 6, 2)) - 1000;
				}
				else if (!manualOperationMode && !autonomous) //When system is not in autonomous mode, to reset speed for joystick release
				{
				speed1 = 0;
				speed2 = 0;
				}
				*/
				/*
				Serial.print("Mode: ");
				Serial.print(payload[8], HEX);
				Serial.print(" Speed1: ");
				Serial.print(speed1);
				Serial.print(" Speed2: ");
				Serial.print(speed2);
				Serial.println();
				*/
				break;
			case CAN_heartbeat:/*
							   id = CAN_heartbeat;
							   len = 1;
							   buf[0] = HEARTBEAT_OCS;
							   ocsHeartbeatTimeout = millis();
							   forwardCANtoSerial(buf);*/
				break;
			default:
				break;
			}
		}

	}

	/**********************************************/
	/*					OCS TX					  */
	/*	Send Telemetry Data via control link	  */
	/**********************************************/

	/**********************************************/
	/*			Transmit Thruster commands		  */
	/**********************************************/


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


//==========================================
//          UPDATE DATA
//==========================================
void reset_stats()
{
	reset_posb_stats();
	reset_ocs_stats();
	reset_rc_stats();
	reset_sbc_stats();
	reset_batt1_stats();
	reset_batt2_stats();
}
void reset_posb_stats()
{
	if (millis() - posb_timeout > HB_TIMEOUT){
		internalStats[INT_PRESS] = 255;
		internalStats[HUMIDITY] = 255;
		internalStats[POSB_TEMP] = 255;
		posb_timeout = millis();
	}
}
void reset_ocs_stats()
{
	if (millis() - ocs_timeout > HB_TIMEOUT){
		internalStats[RSSI_OCS] = 0;
		ocs_timeout = millis();
	}
}
void reset_rc_stats()
{
	rc_timeout = rc.get_last_int_time();
	if (millis() - rc_timeout > HB_TIMEOUT){
		internalStats[RSSI_RC] = 0;
		rc_timeout = millis();
	}
}
void reset_sbc_stats()
{
	if (millis() - sbc_timeout > HB_TIMEOUT){
		internalStats[CPU_TEMP] = 255;
		sbc_timeout = millis();
	}
}
void reset_batt1_stats()
{
	if (millis() - batt1_timeout > HB_TIMEOUT){
		powerStats[BATT1_CAPACITY] = 255;
		powerStats[BATT1_CURRENT] = 255;
		powerStats[BATT1_VOLTAGE] = 255;
		batt1_timeout = millis();
	}
}
void reset_batt2_stats()
{
	if (millis() - batt2_timeout > HB_TIMEOUT){
		powerStats[BATT2_CAPACITY] = 255;
		powerStats[BATT2_CURRENT] = 255;
		powerStats[BATT2_VOLTAGE] = 255;
		batt2_timeout = millis();
	}
}

//==========================================
//          LCD FUNCTIONS
//==========================================

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

//==========================================
//          THRUSTER FUNCTIONS
//==========================================

// Map CPPM values to thruster values
// CPPM: 980 to 2020 (neutral: 1500) (with deadzone from -20 to 20)
// Thruster: -3200 to 3200 (neutral: 0)
int32_t map_cppm(int value)
{
	if(value >= 1500)
	{
		value = constrain(value,1520,2020); // Remove deadzone
		value = map(value, 1520, 2020, 0, 3200);	// Map values
	}
	else
	{
		value = constrain(value,900,1480);
		value = map(value, 980, 1480, -3200, 0);
	}
	return value;
}

void get_directions()
{	
	dir_forward = map_cppm(rc.get_ch(FRISKY_FORWARD));
	dir_side = map_cppm(rc.get_ch(FRISKY_SIDE));
	dir_yaw = map_cppm(rc.get_ch(FRISKY_YAW));
}

void get_controlmode()
{
	if(rc.get_ch(FRISKY_ARM))
	{
		//TODO: resolve ocs arm also
		control_mode = MANUAL_RC;
	}
	else
	{
		control_mode = AUTONOMOUS;
	}
}

// Resolve vector thrust &
// Cap thrust at -3200 and 3200
void convert_thruster_values()
{
	speed1 = constrain(dir_forward - dir_side - dir_yaw, -3200, 3200);
	speed2 = constrain(dir_forward + dir_side + dir_yaw, -3200, 3200);
	speed3 = constrain(dir_forward + dir_side - dir_yaw, -3200, 3200);
	speed4 = constrain(dir_forward - dir_side + dir_yaw, -3200, 3200);
}

//==========================================
//          CAN FUNCTIONS
//==========================================

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

//========== CAN Receive ==============//

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

//========== CAN Transmit ==============//

void publishCAN()
{
	if ((millis() - thruster_loop) > THRUSTER_TIMEOUT)
	{
		publishCAN_manualthruster();
		thruster_loop = millis();
	}
	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		publishCAN_controllink();
		publishCAN_heartbeat();
		heartbeat_loop = millis();
	}
}

void publishCAN_heartbeat()
{	
	buf[0] = HEARTBEAT_Tele;
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

void publishCAN_manualthruster()
{
	CAN.setupCANFrame(buf, 0, 2, (uint32_t)(speed1+3200));
	CAN.setupCANFrame(buf, 2, 2, (uint32_t)(speed2+3200));
	CAN.setupCANFrame(buf, 4, 2, (uint32_t)(speed3+3200));
	CAN.setupCANFrame(buf, 6, 2, (uint32_t)(speed4+3200));
	CAN.sendMsgBuf(CAN_manual_thruster, 0, 8, buf);
}

void publishCAN_controllink()
{	
	id = CAN_control_link;
	len = 1;
	buf[0] = control_mode;
	CAN.sendMsgBuf(CAN_control_link, 0, 1, buf);
}