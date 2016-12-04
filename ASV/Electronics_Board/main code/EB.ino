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
//Firmware Version :             v1.1
////EB Firmware for ASV 1.0
//		SBC to CAN Interface
//		Light Tower
//		Thrusters
//		Kill
//
// Written by Ren Zhi
// Change Log:
// v1.1 - changed to Xbee protocol to adapt to CAN
// v1.2
//###################################################
//###################################################
//###################################################

#include <SPI.h>

#include <can_defines.h>
#include <can.h>
#include <HIH613x.h>

#include "Torqeedo.h"
#include "can_asv_defines.h"
#include "defines.h"
#include "Arduino.h"

//#define _TEST_

MCP_CAN CAN(8);
uint8_t buf[8];
uint32_t id = 0;
uint8_t len = 0;

uint8_t heartbeat;
uint8_t sbc_bus_stats[3];
uint16_t heartbeat_ctr = 0;
uint32_t heartbeat_loop = 0;
uint32_t sbc_bus_loop = 0;
uint32_t time_test = 0;

bool kill_status = false;
bool hard_kill = false;

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop = 0; //250ms loop Publish temp and humidity
static uint32_t thrusterStatsLoop200;
static uint32_t thrusterSpeedLoop100;
static uint32_t lightStatsLoop500;
static uint32_t thrusterHeartbeatLoop200;
static uint32_t ocsHeartbeatTimeout;
uint8_t eb_stat_buf[3];
uint8_t light_num;
uint8_t humid_ctr = 0;

static uint8_t manualOCScontrolBuffer[12];
bool manualOperationMode = false;
int ocsIdx = 0;
byte ocsStart = 0xFF;
uint8_t ocsCANid = 0x00;
int ocslen = 0;

bool ocs_start1 = false;
bool ocs_start2 = false;
bool ocs_stop1 = false;
bool ocs_stop2 = false;


Torqeedo Thruster1(TORQEEDO1_RXEN, TORQEEDO1_DXEN, TORQEEDO1_ON, 1);
Torqeedo Thruster2(TORQEEDO2_RXEN, TORQEEDO2_DXEN, TORQEEDO2_ON, 2);
uint8_t thruster_heartbeat = 0x00;
bool thruster1_batt_heartbeat = false;
bool thruster2_batt_heartbeat = false;
bool thruster1_motor_heartbeat = false;
bool thruster2_motor_heartbeat = false;
int statsState = 0;
int16_t speed1 = 0;
int16_t speed2 = 0;
uint8_t emptybuf[8] = { 0 };
bool speedtest = false;
uint32_t speedtesttimer = 0;
#define SPEEDTESTSIZE 27
#define SPEEDTESTSTEP 1000
int speedtestidx = 0;
int16_t speedtestarray[SPEEDTESTSIZE] =
{0, 500, 500, 500, 500, 500, 500,
0, -1000, -1000, -1000, -1000, -1000, -1000,
0, 500, 500, 500, 500,
-1000, -1000, -1000, -1000, 500, 500, 500, 500};

void setup()
{
	/* add setup code here */

	// CAN BUS INIT
	Serial.begin(115200);
	Serial3.begin(XBEE_BAUDRATE);
	sbc_bus_loop = millis();
	heartbeat_loop = millis();
	Temp_Humid_loop = millis();
	thrusterStatsLoop200 = millis();
	thrusterSpeedLoop100 = millis();
	ocsHeartbeatTimeout = millis();
	speedtesttimer = millis();

START_INIT:
#ifndef _TEST_
	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
	{
		Serial.println("CAN BUS: OK");
	}
	else
	{
		Serial.println("CAN BUS: FAILED");
		Serial.println("CAN BUS: Reinitializing");
		delay(1000);
		goto START_INIT;
	}
	Serial.println("INITIATING TRANSMISSION...");
#endif

	// KILL READ INIT
	initKill();

	// LIGHT TOWER INIT
	initLightTower();
	lightInitSequence();

	// THRUSTER INIT
	Thruster1.init();
	Thruster2.init();
	Serial.flush();
}

uint8_t led_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;

//Serial read frame
int16_t incoming_data = 0;
uint8_t read_flag = 0;
uint8_t read_buffer[8];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;

#ifdef _TEST_
int speed = 0;
char inputstr[10] = { '\n' };
int serialidx = 0;
#endif

void loop()
{
	/*
	/*****************************************/
	/*  Heartbeat - meow					 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

#ifndef _TEST_
	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		//TODO: Needs refactoring
		thruster1_motor_heartbeat = Thruster1.getMotorHeartbeat() & thruster1_batt_heartbeat;
		thruster2_motor_heartbeat = Thruster2.getMotorHeartbeat() & thruster2_batt_heartbeat;

		thruster_heartbeat = thruster1_batt_heartbeat
			+ (thruster2_batt_heartbeat << 1)
			+ (thruster1_motor_heartbeat << 2)
			+ (thruster2_motor_heartbeat << 3);

		thruster1_batt_heartbeat = false;
		thruster2_batt_heartbeat = false;

		id = CAN_heartbeat;
		len = 2;
		buf[0] = HEARTBEAT_EB;
		buf[1] = thruster_heartbeat;
		forwardCANtoSerial(buf);
		heartbeat_loop = millis();
	}
#endif

	/*****************************************/
	/*  Transmit CAN Diagnostics			 */
	/*****************************************/

#ifndef _TEST_
	if ((millis() - sbc_bus_loop) > 1000)
	{
		//CAN.setupCANFrame(sbc_bus_stats, 0, 1, CAN.checkError());
		//CAN.setupCANFrame(sbc_bus_stats, 1, 1, CAN.checkTXStatus(0));
		//CAN.setupCANFrame(sbc_bus_stats, 2, 1, CAN.checkTXStatus(1));
		//CAN.sendMsgBuf(CAN_SBC_BUS_stats, 0, 4, sbc_bus_stats);

		sbc_bus_loop = millis();
	}
#endif

	/*****************************************/
	/*  Check for incoming commands from SBC */
	/*  Transmit commands to CAN Bus		 */
	/*****************************************/
#ifndef _TEST_
	if (Serial.available())
	{
		//read
		while (incoming_data > -1)
		{
			incoming_data = Serial.read();
			if (incoming_data == -1)
			{
				incoming_data = 0;
				break;
			}
			if (incoming_data == START_BYTE && !read_flag)
			{
				read_flag = 1;
				read_ctr = 1;
			}
			else if (incoming_data == START_BYTE && read_flag == 1)
			{
				read_flag++;
			}
			else if (read_flag == 2)
			{
				if (read_ctr == 1)
				{
					read_id = incoming_data;
					read_ctr++;
				}
				else if (read_ctr == 2)
				{
					read_size = incoming_data;
					read_ctr++;
				}
				else if (read_ctr > 2)
				{
					read_buffer[read_ctr - 3] = incoming_data;
					if (read_ctr == (2 + read_size))
					{
						
						switch (read_id)
						{
						case CAN_heartbeat:
							//Check for HEARTBEAT from SBC
							break;
						case CAN_e_stop:
							if (read_buffer[0] && 0x01)
							{
								Thruster1.setKill(true);
								Thruster2.setKill(true);
							}
							else
							{
								Thruster1.setKill(false);
								Thruster2.setKill(false);
							}
							break;
						case CAN_thruster_power:
							id = CAN_thruster_power;
							len = 2;
							buf[0] = 0;
							buf[1] = read_buffer[0];
							forwardCANtoSerial(buf);
							Thruster1.onThruster(read_buffer[0] & 0x01);
							Thruster2.onThruster(read_buffer[0] & 0x02);
							break;
						case CAN_LED:
							if (!kill_status)
								setLightTower(read_buffer[0]);
							break;
						case CAN_thruster:
							//parse speed
							if (!manualOperationMode)
							{
								speed1 = int16_t(CAN.parseCANFrame(read_buffer, 0, 2)) - 1000;
								speed2 = int16_t(CAN.parseCANFrame(read_buffer, 2, 2)) - 1000;
							}
							//Thruster1.setMotorDrive(speed1);
							//Thruster2.setMotorDrive(speed2);
							break;
						default:
							CAN.sendMsgBuf(read_id, 0, read_size, read_buffer);
						}
						read_flag = 0;
						read_ctr = 0;
					}
					else {
						read_buffer[read_ctr - 3] = incoming_data;
						read_ctr++;
					}
				}
			}
		}
	}
#else
	// Manually type in speed through Serial monitor for testing
	if (Serial.available()) {
		byte input = Serial.read();
		inputstr[serialidx] = input;
		Serial.println();
		for (int i = 0; i <= serialidx; i++)
		{
			Serial.print(inputstr[i]);
			Serial.println(i);
		}
		Serial.println();
		if (input == '\n' || input == '\r')
		{
			inputstr[serialidx] = '\0';
			speed = atoi(inputstr);
			if (speed == 5555)
			{
				speed = 0;
				Thruster1.onThruster(false);
				Thruster2.onThruster(false);
			}
			if (speed == 6666)
			{
				speed = 0;
				Thruster1.onThruster(true);
				Thruster2.onThruster(true);
			}
			if (speed == 1111)
			{
				speedtest = !speedtest;
				speed = 0;
			}
			if (speed == 4444)
			{
				if (!Thruster2.kill)
				{
					speed = 0;
					Thruster2.kill = true;
				}
				else
				{
					speed = 0;
					Thruster2.setMotorDrive(0);
					Thruster2.kill = false;
				}
			}
			serialidx = -1;
		}
		serialidx++;
	}
#endif

	/**********************************************/
	/*Check for incoming CAN messages from CAN Bus*/
	/*			 Transmit CAN messages to SBC     */
	/**********************************************/

#ifndef _TEST_
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBufID(&id, &len, buf);// read data,  len: data length, buf: data buf
		forwardCANtoSerial(buf);
	}

	/**********************************************/
	/*        Read humidity and temp sensor       */
	/**********************************************/

	if (millis() - Temp_Humid_loop > HUMID_TIMEOUT)
	{
		switch (humid_ctr)
		{
		case 0:
			humidTempSensor.measurementRequest();
			humid_ctr++;
			break;
		case 1:
			//Get I2C Data
			//push into send state buf
			humidTempSensor.dataFetch();
			eb_stat_buf[0] = humidTempSensor.getTemperature() + 0.5;
			eb_stat_buf[1] = humidTempSensor.getHumidity() + 0.5;
			eb_stat_buf[2] = readKillBattVoltage();
			id = CAN_EB_stats;
			len = 3;
			forwardCANtoSerial(eb_stat_buf);
			humid_ctr = 0;
			Temp_Humid_loop = Temp_Humid_loop + HUMID_TIMEOUT;
			break;
		default:
			break;
		}
	}

#endif

	/**********************************************/
	/* Read OCS XBEE for Manual Thruster override */
	/**********************************************/
	//Serial.print("LOOP");
	if (Serial3.available()) {
		ocsHeartbeatTimeout = millis();
		byte input = Serial3.read();
		switch (input)
		{
		case 0xFE:
			if (ocs_start2)
			{
				manualOCScontrolBuffer[ocsIdx] = input;
				ocsIdx++;
			}
			else
				if (ocs_start1)
				{
					ocs_start2 = true;
					ocsIdx = 0;
				}
				else
					ocs_start1 = true;
			break;
		default:
			ocs_stop1 = false;
			if (ocs_start2)
			{
				switch (ocsIdx)
				{
				case 0:
					ocsCANid = input;
					break;
				case 1:
					ocslen = input;
					break;
				default:
					manualOCScontrolBuffer[ocsIdx] = input;
				}
				ocsIdx++;
			}
			if (ocsIdx > 1 && ocsIdx - 2 >= ocslen)
			{
				//Do shit
				ocs_start1 = false;
				ocs_start2 = false;
				ocsIdx = 0;
				if (ocsCANid == CAN_thruster)
				{
					if (manualOCScontrolBuffer[6]) manualOperationMode = true;
					else manualOperationMode = false;
					speed1 = int16_t(CAN.parseCANFrame(manualOCScontrolBuffer, 2, 2)) - 1000;
					speed2 = int16_t(CAN.parseCANFrame(manualOCScontrolBuffer, 4, 2)) - 1000;
					/*
					Serial.print("Mode: ");
					Serial.print(manualOCScontrolBuffer[6], HEX);
					Serial.print(" Speed1: ");
					Serial.print(speed1);
					Serial.print(" Speed2: ");
					Serial.print(speed2);
					Serial.println();
					*/
				}
				else if (ocsCANid == CAN_heartbeat)
				{
					id = CAN_heartbeat;
					len = 1;
					buf[0] = HEARTBEAT_OCS;
					forwardCANtoSerial(buf);
				}
			}
			break;
		}
	}
	/**********************************************/
	/*     Transmit / Receive Thruster commands   */
	/**********************************************/

	//Safety measure for Loss of Control Link 
	if (manualOperationMode)
	{
		if (millis() - ocsHeartbeatTimeout > OCS_TIMEOUT)
		{
			digitalWrite(LIGHTTOWER_GREEN, HIGH);
			digitalWrite(LIGHTTOWER_YELLOW, HIGH);
			manualOperationMode = 0;
			speed1 = 0;
			speed2 = 0;
		}
	}	

	//Thruster Control per loop
	Thruster1.setMotorDrive(speed1);
	Thruster2.setMotorDrive(speed2);

	// On/off thruster
	if (Thruster1.checkThrusterOnOff())
	{
#ifndef _TEST_
		id = CAN_thruster_power;
		len = 2;
		buf[0] = 0;
		buf[1] = 0x01;
		forwardCANtoSerial(buf);
#endif
	}
	if (Thruster2.checkThrusterOnOff())
	{
#ifndef _TEST_
		id = CAN_thruster_power;
		len = 2;
		buf[0] = 0;
		buf[1] = 0x02;
		forwardCANtoSerial(buf);
#endif
	}

	// Parse thruster commands
	if (Thruster1.readMessage())
		thruster1_batt_heartbeat = true;
	if (Thruster2.readMessage())
		thruster2_batt_heartbeat = true;

	if (millis() - thrusterSpeedLoop100 > 100)
	{
		id = CAN_motor_speed_eb;
		len = 4;
		CAN.setupCANFrame(buf, 0, 2, ((uint32_t)speed1) + 1000);
		CAN.setupCANFrame(buf, 2, 2, ((uint32_t)speed2) + 1000);
		forwardCANtoSerial(buf);
	}

	if (millis() - thrusterStatsLoop200 > 200)
	{
#ifdef _TEST_
		/*
		Serial.print("Mode: ");
		Serial.print(manualOperationMode);
		Serial.print(" Speed1: ");
		Serial.print(speed1);
		Serial.print(" Speed2: ");
		Serial.print(speed2);
		Serial.println();
		*/
#else
		uint8_t *thrusterbuf;
		thrusterbuf = emptybuf;
		switch (statsState)
		{
		case 0:
			id = CAN_thruster1_motor_stats;
			len = 8;
			if (thruster1_batt_heartbeat)
				thrusterbuf = Thruster1.getMotorstats();
			break;
		case 1:
			id = CAN_thruster2_motor_stats;
			len = 8;
			if (thruster2_batt_heartbeat)
				thrusterbuf = Thruster2.getMotorstats();
			break;
		case 2:
			id = CAN_thruster1_battery_stats;
			len = 6;
			if (thruster1_batt_heartbeat)
				thrusterbuf = Thruster1.getBatterystats();
			break;
		case 3:
			id = CAN_thruster2_battery_stats;
			len = 6;
			if (thruster2_batt_heartbeat)
				thrusterbuf = Thruster2.getBatterystats();
			break;
		case 4:
			id = CAN_thruster1_range_stats;
			len = 6;
			if (thruster1_batt_heartbeat)
				thrusterbuf = Thruster1.getRangestats();
			forwardCANtoSerial(thrusterbuf);

			thrusterbuf = emptybuf;
			id = CAN_thruster2_range_stats;
			len = 6;
			if (thruster2_batt_heartbeat)
				thrusterbuf = Thruster2.getRangestats();
			statsState = -1;
			break;
		}
		forwardCANtoSerial(thrusterbuf);
#endif
		statsState++;
		thrusterStatsLoop200 = millis();
}

	/**********************************************/
	/*           Operation Mode light status      */
	/**********************************************/
	if (millis() - lightStatsLoop500 > LIGHTTOWER_TIMEOUT)
	{
		/**********************************************/
		/*           Check Hardware E-Stop            */
		/**********************************************/

		if (digitalRead(KILL_STATUS) == LOW)
		{
			digitalWrite(LIGHTTOWER_RED, HIGH);
		}
		else
		{
			digitalWrite(LIGHTTOWER_RED, LOW);
		}
		if (manualOperationMode)
		{
			//setLightTower(2);
			digitalWrite(LIGHTTOWER_YELLOW, HIGH);
			//digitalWrite(LIGHTTOWER_RED, LOW);
			digitalWrite(LIGHTTOWER_GREEN, LOW);
		}
		else
		{
			digitalWrite(LIGHTTOWER_GREEN, HIGH);
			digitalWrite(LIGHTTOWER_YELLOW, LOW);
			//digitalWrite(LIGHTTOWER_RED, LOW);
		}
		lightStatsLoop500 = millis();
	}
}

//==========================================
//          CAN FUNCTIONS
//==========================================


void forwardCANtoSerial(uint8_t *body)
{
	Serial.write(START_BYTE);
	Serial.write(START_BYTE);
	Serial.write(id);
	Serial.write(len);
	for (int i = 0; i < len; i++)	Serial.write(body[i]);
}

//==========================================
//          KILL FUNCTIONS
//==========================================

void initKill()
{
	pinMode(KILL_BATT, INPUT);
	pinMode(KILL_STATUS, INPUT);
}
uint8_t readKillBattVoltage()
{
	int input = analogRead(KILL_BATT);
	float voltage = ((float)input / 1023)*4.9 * 147 / 27;

	return  uint8_t(voltage * 10);
}

//==========================================
//          LIGHT TOWER FUNCTIONS
//==========================================

void initLightTower()
{
	pinMode(LIGHTTOWER_RED, OUTPUT);
	pinMode(LIGHTTOWER_YELLOW, OUTPUT);
	pinMode(LIGHTTOWER_GREEN, OUTPUT);
	digitalWrite(LIGHTTOWER_RED, LOW);
	digitalWrite(LIGHTTOWER_YELLOW, LOW);
	digitalWrite(LIGHTTOWER_GREEN, LOW);
}

void lightInitSequence()
{
	for (int i = 0; i < 2; i++)
	{
		digitalWrite(LIGHTTOWER_RED, HIGH);
		delay(100);
		digitalWrite(LIGHTTOWER_RED, LOW);
		delay(200);
		digitalWrite(LIGHTTOWER_YELLOW, HIGH);
		delay(100);
		digitalWrite(LIGHTTOWER_YELLOW, LOW);
		delay(200);
		digitalWrite(LIGHTTOWER_GREEN, HIGH);
		delay(100);
		digitalWrite(LIGHTTOWER_GREEN, LOW);
		delay(200);
	}
	return;
}

void setLightTower(byte colour)
{
	if (colour && 0x01)
		digitalWrite(LIGHTTOWER_RED, HIGH);
	else
		digitalWrite(LIGHTTOWER_RED, LOW);

	if (colour && 0x02)
		digitalWrite(LIGHTTOWER_YELLOW, HIGH);
	else
		digitalWrite(LIGHTTOWER_YELLOW, LOW);

	if (colour && 0x04)
		digitalWrite(LIGHTTOWER_GREEN, HIGH);
	else
		digitalWrite(LIGHTTOWER_GREEN, LOW);
}