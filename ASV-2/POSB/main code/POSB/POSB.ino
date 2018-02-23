//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        

// Written by Ng Ren Zhi

// POSB firmware:
//	Thruster control (to ESC via CAN)
//	Battery monitoring (via RS485)
//	Humidity & Temperature
//	Light Tower

// Change Log for v1.0:
// - Initial commit

//###################################################
//###################################################

#include <can_defines.h>
#include <can.h>
#include <HIH613x.h>
#include <Xbee.h>

#include "can_asv_defines.h"
#include "defines.h"
#include "Arduino.h"
#include "Roboteq.h"
#include "Torqeedo.h"

MCP_CAN CAN(8);
uint8_t buf[8];
uint32_t id = 0;
uint8_t len = 0;

Roboteq roboteq1(&CAN, 1);
Roboteq roboteq2(&CAN, 2);
int16_t speed1 = 0;
int16_t speed2 = 0;
int16_t speed3 = 0;
int16_t speed4 = 0;
int control_mode = MANUAL_RC;
int estop_status = false;

Torqeedo Battery1(TORQEEDO1_RXEN, TORQEEDO1_DXEN, TORQEEDO1_ON, 1);
Torqeedo Battery2(TORQEEDO2_RXEN, TORQEEDO2_DXEN, TORQEEDO2_ON, 2);

uint8_t posb_stat_buf[2] = {128,256};
uint8_t esc1_stat_buf[7] = {0x2C, 0x1, 0x14, 0x0, 0, 0, 0}; // 0, 0, 0, 200, 300
uint8_t esc2_stat_buf[7] = {0x90, 0x1, 0xF4, 0x1, 0, 0, 0}; // 0, 0, 0, 500, 400
uint8_t batt1_stat_buf[6] = {25, 0x2C, 0x1, 0x58, 0x2, 75}; // 25, 600, 300, 75
uint8_t batt2_stat_buf[6] = {25, 0x2C, 0x1, 0xBC, 0x2, 50}; // 25, 700, 300, 50
uint32_t heartbeat_loop;
uint32_t batt_loop;
uint32_t windspeed_loop;
uint32_t blink_loop;
uint32_t esc1_loop;
uint32_t esc2_loop;
uint32_t batt1_loop;
uint32_t batt2_loop;
uint32_t power_loop;
uint8_t power_ctr = 0;
uint32_t thruster_loop;
uint32_t thruster_stat_loop;

bool heartbeat_esc1 = false;
bool heartbeat_esc2 = false;
bool heartbeat_batt1 = false;
bool heartbeat_batt2 = false;

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop = 0; //250ms loop Publish temp and humidity
uint8_t humid_ctr = 0;

//#define _TEST_
#ifdef _TEST_

char inputstr[10] = { '\n' };
int serialidx = 0;

#endif

void setup()
{	
	// CAN BUS INIT
	Serial.begin(115200);
	Serial.println("Hi, I'm POSB");
	CAN_init();

	// BATT INIT	
	Battery1.init();
	Battery2.init();
	Serial.flush();

	// ESC INIT
	roboteq1.init();
	roboteq2.init();

	// SENSORS INIT
	Wire.begin();

	// LIGHT INIT
	initLightTower();
	lightInitSequence();

	Temp_Humid_loop = millis();
	heartbeat_loop = millis();
	blink_loop = millis();
	batt_loop = millis();
	windspeed_loop = millis();
	esc1_loop = millis();
	esc2_loop = millis();
	batt1_loop = millis();
	batt2_loop = millis();
	power_loop = millis();
	thruster_loop = millis();
}

void loop()
{
	/************************************/
	/*			Test					*/
	/************************************/
	//Serial.println("RANDOM");
#ifdef _TEST_
	if (Serial.available()) {
		byte input = Serial.read();
		inputstr[serialidx] = input;

		if (input == '\n' || input == '\r')
		{
			inputstr[serialidx] = '\0';

			switch (inputstr[0])
			{
			case 'a':
				roboteq1.requestMotorAmps(2);
				break;
			case 'v':
				roboteq1.requestBatteryVolts();
				break;
			case 'b':
				roboteq1.requestBatteryAmps();
				break;
			case 'k':
				roboteq1.kill();
				break;
			case 'u':
				roboteq1.unkill();
				break;
			case 'f':
				roboteq1.requestFaultFlags();
				break;
			case 'm':
				roboteq1.requestMotorStatusFlags(1);
				break;
			case 'n':
				roboteq1.requestMotorStatusFlags(2);
				break;
			case 'p':
				speed1 = atoi(inputstr + 1);
				Serial.print("Set speed1: ");
				Serial.println(speed1);
				break;
			case 'q':
				speed2 = atoi(inputstr + 1);
				Serial.print("Set speed2: ");
				Serial.println(speed2);
				break;
			}
			serialidx = -1;
			//Serial.println("(a) amps (k) kill (u) unkill");
			//Serial.println("(f) fault flags (m) motor flag 1 (n) motor flag 2");
			//Serial.println("(p[num]) thruster1 (q[num]) thruster2");
		}
		serialidx++;
	}

#endif

	/************************************/
	/*			Thruster Control		*/
	/************************************/

	// Write thruster values to ESC
	if ((millis() - thruster_loop) > THRUSTER_TIMEOUT)
	{
		roboteq1.setMotorSpeed(speed1, 1);
		roboteq1.setMotorSpeed(speed2, 2);
		roboteq2.setMotorSpeed(speed3, 1);
		roboteq2.setMotorSpeed(speed4, 2);
		thruster_loop = millis();
	}
	// Send queries for ESC stats
	if ((millis() - thruster_stat_loop) > THRUSTER_STAT_TIMEOUT)
	{
		roboteq1.requestUpdate();
		roboteq2.requestUpdate();
		thruster_stat_loop = millis();
	}
	// Update heartbeat
	if ((millis() - esc1_loop) > INACTIVITY_TIMEOUT)
	{
		heartbeat_esc1 = false;
		esc1_loop = millis();
	}
	if ((millis() - esc2_loop) > INACTIVITY_TIMEOUT)
	{
		heartbeat_esc2 = false;
		esc2_loop = millis();
	}
	if ((millis() - batt1_loop) > INACTIVITY_TIMEOUT)
	{
		heartbeat_batt1 = false;
		Battery1.resetData();
		batt1_loop = millis();
	}
	if ((millis() - batt2_loop) > INACTIVITY_TIMEOUT)
	{
		heartbeat_batt2 = false;
		Battery2.resetData();
		batt2_loop = millis();
	}

	/************************************/
	/*			Battery Monitoring		*/
	/************************************/
	
	Battery1.checkBatteryOnOff();
	if (Battery1.readMessage())
	{
		heartbeat_batt1 = true;
		batt1_loop = millis();
	}
#define _BATT2_
#ifdef _BATT2_
	Battery2.checkBatteryOnOff();
	if (Battery2.readMessage())
	{
		heartbeat_batt2 = true;
		batt2_loop = millis();
	}
#endif

	// Cycles through status flags, voltage, current
	if((millis() - batt_loop) > BATT_TIMEOUT)
	{
		//Serial.print("REQUEST");
	Battery1.checkBatteryConnected();
	Battery1.requestUpdate();
#ifdef _BATT2_
	Battery2.checkBatteryConnected();
	Battery2.requestUpdate();
#endif
	batt_loop = millis();
	}
	
	/************************************/
	/*	Humidity & Temperature Sensor	*/
	/************************************/

	readTempHumid();

	/************************************/
	/*			Light Tower				*/
	/************************************/

	updateLightTower();

	/************************************/
	/*				CAN					*/
	/************************************/

	// CAN TX
	publishCAN();

	// CAN RX
	checkCANmsg();
}

//========== THRUSTER ====================//

// Map from [0 to 6400] to [-1000 to 1000]
void getThrusterSpeed()
{
	speed1 = map(CAN.parseCANFrame(buf,0,2), 0, 6400, -1000, 1000);
	speed2 = map(CAN.parseCANFrame(buf,2,2), 0, 6400, -1000, 1000);
	speed3 = map(CAN.parseCANFrame(buf,4,2), 0, 6400, -1000, 1000);
	speed4 = map(CAN.parseCANFrame(buf,6,2), 0, 6400, -1000, 1000);
	
	Serial.print(" 1: ");
	Serial.print(speed1);
	Serial.print(" 2: ");
	Serial.print(speed2);
	Serial.print(" 3: ");
	Serial.print(speed3);
	Serial.print(" 4: ");
	Serial.println(speed4);
	
}

//============= LIGHT TOWER =============//


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

// Set light tower to [colour], and turn off all other lights
void setLightTower(byte colour)
{
	if (colour == LIGHTTOWER_RED)
		digitalWrite(LIGHTTOWER_RED, HIGH);
	else
		digitalWrite(LIGHTTOWER_RED, LOW);

	if (colour == LIGHTTOWER_YELLOW)
		digitalWrite(LIGHTTOWER_YELLOW, HIGH);
	else
		digitalWrite(LIGHTTOWER_YELLOW, LOW);

	if (colour == LIGHTTOWER_GREEN)
		digitalWrite(LIGHTTOWER_GREEN, HIGH);
	else
		digitalWrite(LIGHTTOWER_GREEN, LOW);
}

// GREEN: AUTONOMOUS
// GREEN: (BLINK) STATION KEEP
// YELLOW: MANUAL
// RED: ESTOP
void updateLightTower()
{		
	if (estop_status)
	{
		setLightTower(LIGHTTOWER_RED);
	}
	else
	{
		switch (control_mode)
		{
		case AUTONOMOUS:
			setLightTower(LIGHTTOWER_GREEN);
			break;
		case STATION_KEEP:
			if ((millis() - blink_loop) > BLINK_TIMEOUT * 2)
			{
				setLightTower(LIGHTTOWER_NONE);
				blink_loop = millis();
			}
			else if ((millis() - blink_loop) > BLINK_TIMEOUT)
			{
				setLightTower(LIGHTTOWER_GREEN);
			}
			break;
		case MANUAL_OCS:
		case MANUAL_RC:
			setLightTower(LIGHTTOWER_YELLOW);
			break;
		}
	}
}

//======== TEMP HUMID SENSOR =============//

void readTempHumid()
{
	if (millis() - Temp_Humid_loop > HUMID_TIMEOUT)
	{
		switch (humid_ctr)
		{
		case 0:
#define _TEMP_
#ifdef _TEMP_
			humidTempSensor.measurementRequest();
#endif
			humid_ctr++;
			break;
		case 1:
			//Get I2C Data
			//push into send state buf
#ifdef _TEMP_
		{
			humidTempSensor.dataFetch();
			posb_stat_buf[0] = humidTempSensor.getTemperature() + 0.5;
			posb_stat_buf[1] = humidTempSensor.getHumidity() + 0.5;
#endif
			Serial.print("Temp: ");
			Serial.print(posb_stat_buf[0]);
			Serial.print("\t");
			Serial.println(posb_stat_buf[1]);
			publishCAN_posbstats();
			humid_ctr = 0;
			break;
		}
		default:
			break;
		}
			Temp_Humid_loop = millis();
	}
}

//============= CAN =============//

void CAN_init()
{
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
}

void publishCAN()
{
	// Heartbeat
	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		publishCAN_heartbeat();
		heartbeat_loop = millis();
	}
	// Windspeed	
	if ((millis() - windspeed_loop) > WINDSPEED_TIMEOUT)
	{
		publishCAN_windspeed();
		windspeed_loop = millis();
	}
	// POWER: Batt + ESC
	// Cycle btw batt1,2,esc1,2 at 250ms
	if ((millis() - power_loop) > POWER_TIMEOUT)
	{
		switch (power_ctr)
		{
		case 0:
			publishCAN_esc1_stats();
			break;
		case 1:
			publishCAN_esc2_stats();
			break;
		case 2:
			publishCAN_batt1_stats();
			break;
		case 3:
			publishCAN_batt2_stats();
			power_ctr = -1;
			break;
		}
		power_ctr++;
		power_loop = millis();
	}
}

void publishCAN_posbstats()
{
	CAN.sendMsgBuf(CAN_POSB_stats,0,2,posb_stat_buf);
}
void publishCAN_heartbeat()
{
	buf[0] = HEARTBEAT_POSB;
	buf[1] = heartbeat_batt1 + (heartbeat_batt2 << 1) + (heartbeat_esc1 << 2) + (heartbeat_esc2 << 3);
	CAN.sendMsgBuf(CAN_heartbeat,0,2,buf);
}
int wind_dir = 0;
int wind_speed = 0;
void publishCAN_windspeed()
{
	wind_dir = (wind_dir+10)%360;
	wind_speed = (wind_speed + 100) % 1000;
	/*
	Serial.print("WIND: ");
	Serial.print(wind_dir);
	Serial.print(" ");
	Serial.print(wind_speed);*/
	CAN.setupCANFrame(buf,0,2, wind_dir);
	CAN.setupCANFrame(buf,2,2, wind_speed);
	/*
	Serial.print(" | ");
	Serial.print(buf[0],HEX);
	Serial.print(" ");
	Serial.println(buf[1], HEX);*/
	CAN.sendMsgBuf(CAN_wind_speed,0,4,buf);
}
void publishCAN_esc1_stats()
{

	RoboteqStats esc1_stats = roboteq1.getRoboteqStats();
	/*
	Serial.print("Current(A): ");
	Serial.print(esc1_stats.motor_current1);
	Serial.print("\t");
	Serial.print(esc1_stats.motor_current2);
	Serial.print(" Motor flags: ");
	Serial.print(esc1_stats.motor_status_flags1, BIN);
	Serial.print(" ");
	Serial.print(esc1_stats.motor_status_flags2, BIN);
	Serial.print(" Fault flags: ");
	Serial.println(esc1_stats.fault_flags, BIN);*/
	CAN.setupCANFrame(esc1_stat_buf,0,2, esc1_stats.motor_current1);
	CAN.setupCANFrame(esc1_stat_buf,2,2, esc1_stats.motor_current2);
	CAN.setupCANFrame(esc1_stat_buf,3,1, esc1_stats.motor_status_flags1);
	CAN.setupCANFrame(esc1_stat_buf,4,1, esc1_stats.motor_status_flags2);
	CAN.setupCANFrame(esc1_stat_buf,5,1, esc1_stats.fault_flags);
	CAN.sendMsgBuf(CAN_esc1_motor_stats, 0, 7, esc1_stat_buf);
}
void publishCAN_esc2_stats()
{
	RoboteqStats esc2_stats = roboteq2.getRoboteqStats();
	CAN.setupCANFrame(esc2_stat_buf,0,2, esc2_stats.motor_current1);
	CAN.setupCANFrame(esc2_stat_buf,2,2, esc2_stats.motor_current2);
	CAN.setupCANFrame(esc2_stat_buf,3,1, esc2_stats.motor_status_flags1);
	CAN.setupCANFrame(esc2_stat_buf,4,1, esc2_stats.motor_status_flags2);
	CAN.setupCANFrame(esc2_stat_buf,5,1, esc2_stats.fault_flags);
	CAN.sendMsgBuf(CAN_esc2_motor_stats, 0, 7, esc2_stat_buf);
}
void publishCAN_batt1_stats()
{
#ifdef _TEST_
	Serial.print("Capacity(%): ");
	Serial.print(Battery1.getCapacity());
	Serial.print(" Voltage(V): ");
	Serial.print(Battery1.getVoltage());
	Serial.print(" Current(A): ");
	Serial.print(Battery1.getCurrent());
	Serial.print(" Temp(C): ");
	Serial.println(Battery1.getTemperature());
#endif
	/*CAN.setupCANFrame(batt1_stat_buf, 0, 1, 76);
	CAN.setupCANFrame(batt1_stat_buf, 1, 2, 275);
	CAN.setupCANFrame(batt1_stat_buf, 3, 2, 0 - (int16_t)(-14));
	CAN.setupCANFrame(batt1_stat_buf, 5, 1, 38);
	CAN.sendMsgBuf(CAN_battery1_motor_stats, 0, 6, batt1_stat_buf);*/
	CAN.setupCANFrame(batt1_stat_buf,0,1, Battery1.getCapacity());
	CAN.setupCANFrame(batt1_stat_buf,1,2, Battery1.getVoltage());
	CAN.setupCANFrame(batt1_stat_buf,3,2, 0-(int16_t)Battery1.getCurrent());
	CAN.setupCANFrame(batt1_stat_buf,5,1, Battery1.getTemperature());
	CAN.sendMsgBuf(CAN_battery1_motor_stats, 0, 6, batt1_stat_buf);
}
void publishCAN_batt2_stats()
{
	CAN.setupCANFrame(batt2_stat_buf,0,1, Battery2.getCapacity());
	CAN.setupCANFrame(batt2_stat_buf,1,2, Battery2.getVoltage());
	CAN.setupCANFrame(batt2_stat_buf,3,2, 0 - (int16_t)Battery2.getCurrent());
	CAN.setupCANFrame(batt2_stat_buf,5,1, Battery2.getTemperature());
	CAN.sendMsgBuf(CAN_battery2_motor_stats, 0, 6, batt2_stat_buf);
}
void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		switch(id){
#ifndef _TEST_
		case CAN_thruster:
			Serial.print(control_mode);
			if((control_mode == AUTONOMOUS) || (control_mode == STATION_KEEP))
			{
				Serial.print(" AUTO: ");
				getThrusterSpeed();
			}
			break;
		case CAN_manual_thruster:
				Serial.print(control_mode);
			if((control_mode == MANUAL_OCS) || (control_mode == MANUAL_RC))
			{
				Serial.print(" MANUAL: ");
				getThrusterSpeed();
			}
			break;
#endif
		case CAN_control_link:
			control_mode = CAN.parseCANFrame(buf,0,1);
			break;
		case CAN_e_stop:
			estop_status = CAN.parseCANFrame(buf, 0, 1);
			break;
		case ROBOTEQ_CAN1_REPLY_INDEX:
			roboteq1.readRoboteqReply(id, len, buf);
			heartbeat_esc1 = true;
			esc1_loop = millis();
			break;
		case ROBOTEQ_CAN2_REPLY_INDEX:
			roboteq2.readRoboteqReply(id, len, buf);
			heartbeat_esc2 = true;
			esc2_loop = millis();
			break;
		default:
			//Serial.println("Others");
			break;
		}
		/*
		Serial.print("ID: ");
		Serial.print(id);
		Serial.print(" Len: ");
		Serial.print(len);
		Serial.print(" Payload: ");
		for(int i = 0; i < len; i++)
		{
		Serial.print(CAN.parseCANFrame(buf,i,1),HEX);
		Serial.print(" ");
		}
		Serial.println();
		*/
		CAN.clearMsg();
	}
}