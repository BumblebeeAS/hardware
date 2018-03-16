//###################################################
//###################################################
//
//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        
//
//Telemetry for BBASV 2.0
//Firmware Version :             v1.0
//Telemetry Firmware for ASV 2.0
//		Telemetry LCD Display
//		Frisky Receiver (CPPM & DAC)
//		XBee
//
// Written by Ren Zhi and Chia Che
// Change log v0.0:
//
//###################################################
//###################################################
//###################################################

#include <Wire.h>
#include "XBee.h"
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include "LCD_Driver.h"
#include "Frisky_CPPM.h"
#include "define.h"
#include <can_defines.h>
#include "can_asv_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>

#define _XBEE_
LCD screen = LCD(SCREEN_CS, SCREEN_RESET);  //screen
Frisky rc = Frisky(RC_INT);

MCP_CAN CAN(CAN_Chip_Select);

uint32_t id = 0;
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
static uint32_t currentTime;
static uint32_t canStatsTime;

static uint32_t heartbeat_loop = 0;
static uint32_t thruster_loop = 0;
static uint32_t rc_loop = 0;

int control_mode = AUTONOMOUS;
int control_mode_rc = AUTONOMOUS;
int control_mode_ocs = AUTONOMOUS;
int16_t dir_forward = 0;
int16_t dir_side = 0;
int16_t dir_yaw = 0;
int16_t speed1 = 0;
int16_t speed2 = 0;
int16_t speed3 = 0;
int16_t speed4 = 0;

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, OCS_EXT);
ZBTxRequest zbTx;
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
uint8_t *payload;
uint8_t xbee_buf[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
uint8_t cmd[] = { 'D','B' };
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

int count = 0;

void setup() {
	Serial.begin(115200);
	Serial.println("Hi, I'm telemetry!");

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
	Serial.println("INITIATING TRANSMISSION...");

	// DAC INIT
	Wire.begin();

	currentTime = loopTime = millis();
	for (int i = 0; i < HB_COUNT; i++)
	{
		heartbeat_timeout[i] = millis();
	}
}


void loop() {
	//******* LCD SCREEN **********/

	reset_stats();

	if ((millis() - loopTime) > SCREEN_LOOP) {
		screen_update();
		loopTime = millis();
		update_heartbeat();

		id = 5;
		len = 3;
		buf[0] = 1;
		buf[1] = 2;
		buf[2] = 3;
		//forwardToXbee();
	}

	/*************************************************/
	/*					RC RX					     */
	/* Read Frisky CPPM for Manual Thruster override */
	/*************************************************/
	get_controlmode_rc();
	get_rssi();
	if (control_mode == MANUAL_RC)
	{
		get_directions();

#ifdef _DEBUG_
		Serial.print("MANUAL RC -");
		Serial.print(" 1: ");
		Serial.print(speed1);
		Serial.print(" 2: ");
		Serial.print(speed2);
		Serial.print(" 3: ");
		Serial.print(speed3);
		Serial.print(" 4: ");
		Serial.println(speed4);
	}
	else if (control_mode == AUTONOMOUS)
	{
		Serial.println("AUTONOMOUS");
	}
	else
	{
		Serial.println("STATION KEEP");
#endif
	}

	/********************************************/
	/*					RC TX				    */
	/* Send ASV Batt value to Frisky Controller */
	/********************************************/
	uint16_t lower_batt = min(powerStats[BATT1_CAPACITY], powerStats[BATT2_CAPACITY]);
	//TODO: min of 2 gives 0 if one of the batt is not connected
	uint16_t DAC_input = convert_batt_capacity(lower_batt);
	//TODO set DAC refresh every 2s	
	set_DAC(DAC_input);

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
			len = rx.getDataLength();
			/*
			for (int i = 0; i < len; i++)
			{
				Serial.print(payload[i]);
				Serial.print(" ");
			}
			Serial.print(" ||| id: ");
			Serial.print(payload[2]);
			Serial.print(" | len: ");
			Serial.print(payload[3]);
			Serial.print(" | data: ");
			for (int i = 4; i < len; i++)
			{
				Serial.print(payload[i]);
				Serial.print(" ");
			}
			Serial.println("");*/
			switch (payload[2])
			{
			case CAN_control_link:
				control_mode_ocs = payload[4];
				internalStats[RSSI_OCS] = payload[6];
				break;
			case CAN_manual_thruster:
					Serial.print("OCS Mode: ");
					Serial.print(control_mode_ocs);
				if (control_mode == MANUAL_OCS)
				{
					speed1 = (uint16_t(CAN.parseCANFrame(payload, 4, 2)))-3200;
					speed2 = (uint16_t(CAN.parseCANFrame(payload, 6, 2)))-3200;
					speed3 = (uint16_t(CAN.parseCANFrame(payload, 8, 2)))-3200;
					speed4 = (uint16_t(CAN.parseCANFrame(payload, 10, 2)))-3200;
					/*
					Serial.print(" Speed1: ");
					Serial.print(speed1);
					Serial.print(" Speed2: ");
					Serial.print(speed2);*/
					
				}
					Serial.println();
				break;
			case CAN_heartbeat:
				if (payload[4] == HEARTBEAT_OCS)
				{
					publishCAN_heartbeat(HEARTBEAT_OCS);
					heartbeat_timeout[HEARTBEAT_OCS] = millis();
				}
				break;
			case CAN_soft_e_stop:
				forwardToCAN(payload);
				break;
			case CAN_POPB_control:
				forwardToCAN(payload);
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

	// TODO: Send data to control link
	// Now integrated to into each checkCANmsg

	/**********************************************/
	/*			Transmit Thruster commands		  */
	/**********************************************/
	get_controlmode();
	set_thruster_values();

	//******* CAN RX **********/
	checkCANmsg();

	//******* CAN TX **********/
	publishCAN();

	//******* CAN ERROR REPORTING **********/
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
Internal Pressure
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
	if ((millis() - posb_timeout) > STAT_TIMEOUT) {
		internalStats[INT_PRESS] = 255;
		internalStats[HUMIDITY] = 255;
		internalStats[POSB_TEMP] = 255;
		posb_timeout = millis();
	}
}
void reset_ocs_stats()
{
	if ((millis() - ocs_timeout) > STAT_TIMEOUT) {
		internalStats[RSSI_OCS] = 0;
		ocs_timeout = millis();
	}
}
void reset_rc_stats()
{
	rc_timeout = rc.get_last_int_time(); // rc_timeout is in micros
	if ((micros() - rc_timeout) > STAT_TIMEOUT * 1000) {
		internalStats[RSSI_RC] = 0;
		rc.reset();
		rc_timeout = micros();
	}
}
void reset_sbc_stats()
{
	if ((millis() - sbc_timeout) > STAT_TIMEOUT) {
		internalStats[CPU_TEMP] = 255;
		sbc_timeout = millis();
	}
}
void reset_batt1_stats()
{
	if ((millis() - batt1_timeout) > STAT_TIMEOUT) {
		powerStats[BATT1_CAPACITY] = 255;
		powerStats[BATT1_CURRENT] = 255;
		powerStats[BATT1_VOLTAGE] = 255;
		batt1_timeout = millis();
	}
}
void reset_batt2_stats()
{
	if ((millis() - batt2_timeout) > STAT_TIMEOUT) {
		powerStats[BATT2_CAPACITY] = 255;
		powerStats[BATT2_CURRENT] = 255;
		powerStats[BATT2_VOLTAGE] = 255;
		batt2_timeout = millis();
	}
}

//==========================================
//          LCD FUNCTIONS
//==========================================

void screen_prepare() {
	screen.set_cursor(0, 0);
	screen.write_string("Int press:");
	screen.write_string("Humidity:");
	screen.write_string("CPU temp:");
	screen.write_string("POSB temp:");
	screen.write_string("RSSI OCS:");
	screen.write_string("RSSI RC:");
	screen.write_string("POSB OK:");
	screen.write_string("POPB OK:");
	screen.write_string("POKB OK:");
	screen.write_string("LARS OK:");
	screen.write_string("SBC OK:");
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
}

void screen_update() {
	screen.set_cursor(150, 0);
	for (int i = 0; i < INT_STAT_COUNT; i++)
	{
		screen.write_value_int(internalStats[i]);
	}

	screen.set_cursor(645, 0);
	for (int i = 0; i < POWER_STAT_COUNT; i++)
	{
		screen.write_value_int(powerStats[i]);
	}
	//screen.write_value_int(count);
	//count = (count + 1) % 10;
}

void update_heartbeat()
{
	int i;
	/* CHECK FOR HEARTBEAT */
	screen.set_cursor(150, 210);
	for (i = 1; i < 9; i++) {
		if (i != HEARTBEAT_Tele) // Skip Telemetry HB
		{
			if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
				screen.write_value_string("NO");
			}
			else
				screen.write_value_string("YES");
		}
	}

	screen.set_cursor(550, 210);
	for (; i < 13; i++) {
		if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
			screen.write_value_string("NO");
		}
		else
			screen.write_value_string("YES");
	}
}

//==========================================
//          THRUSTER FUNCTIONS
//==========================================

// Map CPPM values to thruster values
// CPPM: 980 to 2020 (neutral: 1500) (with deadzone from -24 to 24)
// Thruster: -3200 to 3200 (neutral: 0)
int32_t map_cppm(uint32_t value)
{
	value = remove_deadzone(value);
	if (value >= 1500)
	{
		value = map(value, 1524, 2020, 0, 3200);	// Map values
	}
	else
	{
		value = map(value, 980, 1476, -3200, 0);
	}
	return value;
}

// Remove deadzone around 1500 (from +24 to -24)
uint32_t remove_deadzone(uint32_t value)
{
	if (value >= 1500)
	{
		return constrain(value, 1524, 2020);
	}
	else
	{
		return constrain(value, 980, 1476);
	}

}

void get_directions()
{
	dir_forward = map_cppm(rc.get_ch(FRISKY_FORWARD));
	dir_side = map_cppm(rc.get_ch(FRISKY_SIDE));
	dir_yaw = map_cppm(rc.get_ch(FRISKY_YAW));
}

// Resolve vector thrust &
// Cap thrust at -3200 and 3200
void set_thruster_values()
{
	if ((millis() - heartbeat_timeout[HEARTBEAT_POKB]) > FAILSAFE_TIMEOUT)
	{
		// If no POKB heartbeat, stop all thrusters
		reset_thruster_values();
	}
	else
	{
		switch (control_mode)
		{
		case AUTONOMOUS:
		case STATION_KEEP:
			// If not in manual mode, reset all thrusters
			reset_thruster_values();
			break;
		case MANUAL_RC:
			convert_thruster_values();
			break;
		}
	}
}
// Resolve vector thrust &
// Cap thrust at -3200 and 3200
void convert_thruster_values()
{
			speed1 = constrain(-dir_forward - dir_side - dir_yaw, -3200, 3200);
			speed2 = constrain(-dir_forward + dir_side + dir_yaw, -3200, 3200);
			speed3 = constrain(dir_forward - dir_side + dir_yaw, -3200, 3200);
			speed4 = constrain(dir_forward + dir_side - dir_yaw, -3200, 3200);
}
void reset_thruster_values()
{
	speed1 = 0;
	speed2 = 0;
	speed3 = 0;
	speed4 = 0;
}


//==========================================
//          FRISKY FUNCTIONS
//==========================================

void get_rssi()
{
	// Map RSSI from 1500 to 2000 duty cycle to 0 to 100 dB
	internalStats[RSSI_RC] = calculate_rssi();
	if ((internalStats[RSSI_RC] != 255) && (internalStats[RSSI_RC] > RSSI_THRESHOLD))
	{
		heartbeat_timeout[HEARTBEAT_RC] = millis();
	}
}

int calculate_rssi()
{
	// WHY DEADZONE
	// Remove deadzone, remove values below 1500
	// and map from [1500 to 2000] to [0 to 100]
	uint32_t dead = remove_deadzone(rc.get_ch(FRISKY_RSSI));
	int cppm = constrain(dead, 1524, 2000);
	return map(cppm, 1524, 2000, 0, 100);
}

void get_controlmode()
{
	if (((millis() - heartbeat_timeout[HEARTBEAT_Cogswell]) > FAILSAFE_TIMEOUT)
		|| ((millis() - heartbeat_timeout[HEARTBEAT_POKB]) > FAILSAFE_TIMEOUT)) // Lost SBC or POKB heartbeat
	{
		if (control_mode_rc != AUTONOMOUS) // rc overrides ocs
		{
			control_mode = control_mode_rc;
		}
		else
		{
			control_mode = MANUAL_OCS;
		}
	}
	else if (((millis() - heartbeat_timeout[HEARTBEAT_RC]) > COMMLINK_TIMEOUT) &&
		((millis() - heartbeat_timeout[HEARTBEAT_OCS]) > COMMLINK_TIMEOUT)) // Both rc & ocs loss comms
	{
		control_mode = STATION_KEEP;
	}
	else if (control_mode_rc != AUTONOMOUS) // rc overrides ocs
	{
		control_mode = control_mode_rc;
	}
	// If ocs is alive and not autonomous
	else if (((millis() - heartbeat_timeout[HEARTBEAT_OCS]) < COMMLINK_TIMEOUT) && (control_mode_ocs != AUTONOMOUS))
	{
		control_mode = control_mode_ocs;
	}
	else
	{
		control_mode = AUTONOMOUS;
	}
}

void get_controlmode_rc()
{
	if (rc.get_ch(FRISKY_ARM) > 1800)
	{
		control_mode_rc = MANUAL_RC;
	}
	else if (rc.get_ch(FRISKY_ARM) > 1200)
	{
		control_mode_rc = STATION_KEEP;
	}
	else
	{
		control_mode_rc = AUTONOMOUS;
	}
}

uint16_t convert_batt_capacity(uint32_t capacity)
{
	float scaled_capacity = capacity * 3.3 / 100;
	uint16_t DAC_input = scaled_capacity * 4096 / 5;
	return DAC_input;
}

// Write 12-bit input to DAC 
void set_DAC(uint16_t DACinput)
{
	Wire.beginTransmission(I2C_ADDR_DAC);
	Wire.write(DACinput >> 8);     // top 4 bit of the 12bit voltage
	Wire.write(DACinput & 0xFF);    // bot 8 bit of the 12bit voltage
	Wire.endTransmission(true);
}

//==========================================
//          CAN FUNCTIONS
//==========================================

void CAN_init() {
START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 1000k
#if DEBUG_MODE == NORMAL
		Serial.println("CAN init ok!");
#endif           
	}
	else {
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

void checkCANmsg() {
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		switch (CAN.getCanId()) {
		case CAN_heartbeat:
		{
			uint32_t device = CAN.parseCANFrame(buf, 0, 1);
			//Serial.print(" heartbeat: ");
			//Serial.println(device);
			heartbeat_timeout[device] = millis();
			get_thruster_batt_heartbeat();
			break;
		}

		case CAN_battery1_stats:
			//Serial.println("Batt1 stats");
			powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
			powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
			powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 3, 2);

			batt1_timeout = millis();
			break;

		case CAN_battery2_stats:
			//Serial.println("Batt2 stats");
			powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
			powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
			powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 3, 2);
			batt2_timeout = millis();
			break;

		case CAN_cpu_temp:
			//Serial.println("sbc stats");
			internalStats[CPU_TEMP] = CAN.parseCANFrame(buf, 0, 1);
			sbc_timeout = millis();
			break;

		case CAN_POSB_stats:
			//Serial.println("posb stats");
			//internalStats[INT_PRESS] = CAN.parseCANFrame(buf, 2, 1);
			internalStats[INT_PRESS] = 255;
			internalStats[HUMIDITY] = CAN.parseCANFrame(buf, 1, 1);
			internalStats[POSB_TEMP] = CAN.parseCANFrame(buf, 0, 1);
			posb_timeout = millis();
			break;

		default:
			//Serial.println("Others");
			break;
		}
		switch (CAN.getCanId()) {
		case CAN_heartbeat:
		case CAN_e_stop:
		case CAN_wind_speed:
		case CAN_battery1_stats:
		case CAN_battery2_stats:
		case CAN_esc1_motor_stats:
		case CAN_esc2_motor_stats:
		case CAN_remote_kill_stats:
		case CAN_INS_stats:
		case CAN_GPS_stats:
		case CAN_cpu_temp:
		case CAN_POSB_stats:
		case CAN_POPB_stats:
			CAN.parseCANFrame(buf, 0, len);
#ifdef _XBEE_
			forwardToXbee();
#endif
			break;
		default:
			break;
		}

		CAN.clearMsg();
	}
}

// Extract ESC and battery heartbeats
void get_thruster_batt_heartbeat()
{
	if (len == 2) // if is POSB heartbeat
	{
		uint8_t thruster_heartbeat = CAN.parseCANFrame(buf, 1, 1);
		/*Serial.print("ESC + Thruster HB: ");
		Serial.println(thruster_heartbeat, HEX);*/
		for (int i = 0; i < 4; i++)
		{
			if (thruster_heartbeat & 1) // Check first bit
			{
				heartbeat_timeout[BATT1 + i] = millis();
				//Serial.print(BATT1 + i);
			}
			//Serial.println("");
			thruster_heartbeat = thruster_heartbeat >> 1; // Move to next bit
		}
	}
}

//========== CAN Transmit ==============//

void publishCAN()
{
	if (((control_mode == MANUAL_RC) || (control_mode == MANUAL_OCS)) &&
		((millis() - thruster_loop) > THRUSTER_TIMEOUT))
	{
		publishCAN_manualthruster();
		thruster_loop = millis();
	}
	if ((millis() - heartbeat_loop) > HEARTBEAT_LOOP)
	{
		publishCAN_controllink();
#ifdef _XBEE_
		forwardToXbee(); // Fwd controllink msg
#endif
		publishCAN_heartbeat(HEARTBEAT_Tele);
#ifdef _XBEE_
		forwardToXbee(); // Fwd tele heartbeat msg 
#endif
		heartbeat_loop = millis();
	}
	if ((internalStats[RSSI_RC] > RSSI_THRESHOLD) &&	// If no rc link (low rssi), don't send heartbeat
		((millis() - rc_loop) > HEARTBEAT_LOOP))
	{
		publishCAN_heartbeat(HEARTBEAT_RC);
		rc_loop = millis();
	}
}

void publishCAN_heartbeat(int device_id)
{
	buf[0] = device_id;
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

void publishCAN_manualthruster()
{
	CAN.setupCANFrame(buf, 0, 2, (uint32_t)(speed1 + 3200));
	CAN.setupCANFrame(buf, 2, 2, (uint32_t)(speed2 + 3200));
	CAN.setupCANFrame(buf, 4, 2, (uint32_t)(speed3 + 3200));
	CAN.setupCANFrame(buf, 6, 2, (uint32_t)(speed4 + 3200));
	CAN.sendMsgBuf(CAN_manual_thruster, 0, 8, buf);
}

void publishCAN_controllink()
{
	id = CAN_control_link;
	len = 3;
	buf[0] = control_mode;
	buf[1] = internalStats[RSSI_RC];
	/*buf[2] = getXbeeRssi();
	Serial.print("RSSI: ");
	Serial.println(buf[2]);*/
	buf[2] = internalStats[RSSI_OCS];
	CAN.sendMsgBuf(CAN_control_link, 0, 3, buf);
}

void forwardToCAN(uint8_t payload[])
{
	id = payload[2];
	len = payload[3];
	for (int i = 0; i < len; i++)
	{
		buf[i] = payload[4 + i];
	}
	CAN.sendMsgBuf(id, 0, len, buf);
}

//==========================================
//          XBEE FUNCTIONS
//==========================================

void forwardToXbee() {
	//START_BYTE x2, len, id, buf
	zbTx = ZBTxRequest(addr64, xbee_buf, 4 + len);
	xbee_buf[0] = START_BYTE;
	xbee_buf[1] = START_BYTE;
	xbee_buf[2] = id;
	xbee_buf[3] = len;
	for (int i = 4, j = 0; j < len; i++, j++) {
		// Copy data from CAN buf to xbee buf
		xbee_buf[i] = buf[j];
	}
	xbee.send(zbTx);
	if (xbee.readPacket(100))
	{
		// got a response!
		// should be a znet tx status      
		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
		{
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			// get the delivery status, the fifth byte
			if (txStatus.getDeliveryStatus() == SUCCESS)
			{
				// success.  time to celebrate
				//Serial.println("Ack");
			}
			else
			{
				//Serial.println("No Acknowledgement");
			}
		}
		else
		{
			// local XBee did not provide a timely TX Status Response -- should not happen
			Serial.println("Sender Error");
		}
	}
}

// Not used
// XBee RSSI retrieved from ocs side instead
uint8_t getXbeeRssi() {
	atRequest.setCommand(cmd);
	xbee.send(atRequest);

	// wait up to 5 seconds for the status response
	if (xbee.readPacket(100)) {
		// got a response!

		// should be an AT command response
		if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			xbee.getResponse().getAtCommandResponse(atResponse);

			if (atResponse.isOk()) {
				return atResponse.getValue()[0];
			}
		}
	}
}
