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
//Firmware Version :             v1.0
////EB Firmware for ASV 1.0
//		SBC to CAN Interface
//		Light Tower
//		Thrusters
//		Kill
//
// Written by Ren Zhi
// Change Log:
// NIL
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
//#include <iostream>

#define _TEST_

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

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop250; //250ms loop Publish temp and humidity
static uint32_t Temp_Humid_requestloop250; //250ms loop request temp and humidity
static uint32_t thrusterStatsLoop200;

char manualOCScontrolBuffer[12];
bool manualOperationMode = false;
int ocsIdx = 0;
byte ocsStart = 0xFF;

bool ocs_start1 = false;
bool ocs_start2 = false;
bool ocs_stop1 = false;
bool ocs_stop2 = false;


Torqeedo Thruster1(TORQEEDO1_RXEN, TORQEEDO1_DXEN, TORQEEDO1_ON, 1);
Torqeedo Thruster2(TORQEEDO2_RXEN, TORQEEDO2_DXEN, TORQEEDO2_ON, 2);
int statsState = 0;
int8_t speed1_8 = 0;
int8_t speed2_8 = 0;
int16_t speed1 = 0;
int16_t speed2 = 0;


void setup()
{
	/* add setup code here */

	// CAN BUS INIT
	Serial.begin(115200);
	Serial3.begin(XBEE_BAUDRATE);
	sbc_bus_loop = millis();
	heartbeat_loop = millis();
	Temp_Humid_loop250 = millis();
	Temp_Humid_requestloop250 = millis() + 250;
	thrusterStatsLoop200 = millis();
START_INIT:

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

	// LIGHT TOWER INIT
	initLightTower();

	// THRUSTER INIT
	Thruster1.init();
	Thruster2.init();
}

uint8_t led_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;

//Serial read frame
int16_t incoming_data = 0;
//uint8_t read_flag = 0;
uint8_t read_buffer[8];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;

#ifdef _TEST_
int speed = 0;
char inputstr[10] = {};
int serialidx = 0;
#endif

void loop()
{
	/*
	/*****************************************/
	/*  Heartbeat							 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

#ifndef _TEST_
	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		CAN.setupCANFrame(buf, 0, 0, HEARTBEAT_SBC_CAN);
		buf[0] = HEARTBEAT_SBC_CAN;
		CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
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
							//Serial.println("hb!");
							break;
						case CAN_LED:
							setLightTower(read_buffer[0]);
							break;
						case CAN_thruster:
							if (!manualOperationMode)
							{
								//parse speed
								int16_t speed1 = (read_buffer[1] << 8) | (read_buffer[0]);
								int16_t speed2 = (read_buffer[3] << 8) | (read_buffer[2]);
								Thruster1.setMotorDrive(speed1);
								Thruster2.setMotorDrive(speed2);
							}
							break;
						default:
							CAN.sendMsgBuf(read_id, 0, read_size, read_buffer);
							read_flag = 0;
							read_ctr = 0;
						}
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
	serialidx = 0;
	while (Serial.available()) {
		byte input = Serial.read();
		inputstr[serialidx] = input;
		serialidx++;
		delay(1);
	}
	if (serialidx > 0)
	{
		inputstr[serialidx] = '\0';
		speed = atoi(inputstr);
		if (speed == 5555)
		{
			speed = 0;
			Thruster2.setMotorDrive(0);
			digitalWrite(TORQEEDO2_ON, LOW);
			delay(3500);
			Thruster2.startUpCount = 0;
			digitalWrite(TORQEEDO2_ON, HIGH);
		}
		if (speed == 4444)
		{
			if (!Thruster2.kill)
			{
				Thruster2.kill = true;
			}
			else
			{
				speed = 0;
				Thruster2.setMotorDrive(0);
				Thruster2.kill = false;
			}
		}
	}
	if (!manualOperationMode)
	{
		Thruster2.setMotorDrive(speed);
	}
#endif

	/**********************************************/
	/*Check for incoming CAN messages from CAN Bus*/
	/*			 Transmit CAN messages to SBC     */
	/**********************************************/


	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBufID(&id, &len, buf);// read data,  len: data length, buf: data buf
		//forwardCANmsg();
	}

	/**********************************************/
	/*        Read humidity and temp sensor       */
	/**********************************************/
	if (millis() - Temp_Humid_requestloop250 > 250)
	{
		humidTempSensor.measurementRequest();
		Temp_Humid_requestloop250 = millis();
		Temp_Humid_loop250 = millis();
	}
	if (millis() - Temp_Humid_loop250 > 25)
	{
		//Get I2C Data
		//push into send state buf
		humidTempSensor.dataFetch();
		//somebuf[0] = humidTempSensor.getTemperature();
		//somebuf[1] = humidTempSensor.getHumidity();
		//id = CAN_THRUSTER_STATS;
		//forwardCANmsg();
		Temp_Humid_loop250 = millis() + 250;
	}

	/**********************************************/
	/* Read OCS XBEE for Manual Thruster override */
	/**********************************************/
	//Serial.print("LOOP");
	if (Serial3.available()) {
		byte input = Serial3.read();
		switch (input)
		{
		case 0x2F:
			ocs_stop1 = false;
			if (ocs_start2)
			{
				manualOCScontrolBuffer[ocsIdx] = input;
				//Serial.print(input, HEX); Serial.print(" ");
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
		case 0x4F:
			if (ocs_start2)
				if (!ocs_stop1)
					ocs_stop1 = true;
				else
				{
					//Do shit
					ocs_start1 = false;
					ocs_start2 = false;
					ocs_stop1 = false;
					if (ocsIdx == 3)
					{
						ocsIdx = 0;
						if (manualOCScontrolBuffer[0] == 0x01)
							manualOperationMode = true;
						else
							manualOperationMode = false;
						speed1 = (int8_t)(manualOCScontrolBuffer[1] - 127);
						speed2 = (int8_t)(manualOCScontrolBuffer[2] - 127);
						//for(int i = 0; i < 3; i++)
						//	Serial.print(manualOCScontrolBuffer[i], HEX); Serial.print(" ");
						
						Serial.print("Mode: ");
						Serial.print(manualOCScontrolBuffer[0], HEX);
						Serial.print(" Speed1: ");
						Serial.print(speed1);
						Serial.print(" Speed2: ");
						Serial.print(speed2);
						Serial.print("\n");
						
					}
				}
			break;
		default:
			ocs_stop1 = false;
			if (ocs_start2)
			{
				manualOCScontrolBuffer[ocsIdx] = input;
				//Serial.print(input, HEX); Serial.print(" ");
				ocsIdx++;
			}
			break;
		}
	}
	/*
	if (Serial.available())
	{
		while (Serial3.available() && ocsIdx < 12)
		{
			manualOCScontrolBuffer += Serial3.read();
			if (manualOCScontrolBuffer.back() == '\n')
			{
				if (manualOCScontrolBuffer[0] == MANUAL_OPERATION)
				{
					manualOperationMode = true;
				}
				else
				{
					manualOperationMode = false;
				}

				speed1 = std::atoi(manualOCScontrolBuffer.substr(1, 5));
				speed2 = std::atoi(manualOCScontrolBuffer.substr(6, 5));

				Serial.println(manualOCScontrolBuffer);

				manualOCScontrolBuffer = "";
			}
		}
	}
	*/
	//Serial.println("LOOP");

	/**********************************************/
	/*     Transmit / Receive Thruster commands   */
	/**********************************************/

	// Manual Operation Override
	if (manualOperationMode)
	{
		/*
		Serial.print("Manual 1: ");
		Serial.print(speed1);
		Serial.print(" | 2: ");
		Serial.println(speed2);*/
		Thruster1.setMotorDrive(speed1);
		Thruster2.setMotorDrive(speed2);
	}

	//Thruster1.readMessage(available1, read1);
	Thruster2.readMessage(available2, read2);
	if (millis() - thrusterStatsLoop200 > 100)
	{
		
		Serial.print("Mode: ");
		Serial.print(manualOperationMode);
		Serial.print(" Speed1: ");
		Serial.print(speed1);
		Serial.print(" Speed2: ");
		Serial.print(speed2);
		Serial.print("\n");
		switch (statsState)
		
		{
		case 0:
			//Thruster1.getMotorstats();
			break;
		case 1:
			//Thruster2.getMotorstats();
			break;
		case 2:
			//Thruster1.getBatterystats();
			break;
		case 3:
			//Thruster1.getBatterystats();
			break;
		case 4:
			//Thruster1.getRangestats();
			//Thruster1.getRangesStats();
			statsState = 0;
		}
		id = CAN_thruster1_battery_stats;
		//forwardCANmsg();
		thrusterStatsLoop200 = millis();
	}
}

//==========================================
//          CAN FUNCTIONS
//==========================================

void forwardCANmsg()
{
	Serial.write(START_BYTE);
	Serial.write(START_BYTE);
	Serial.write(id);
	Serial.write(len);
	for (int i = 0; i < len; i++)	Serial.write(buf[i]);
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