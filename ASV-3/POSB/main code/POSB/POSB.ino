//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/    chrome     

// Written by Ng Ren Zhi
// Edited by Justin

// POSB firmware:
//	Thruster control (to ESC via CAN)
//	Battery monitoring (via RS485)
//	Humidity & Temperature & Internal Pressure & Leak Sensor
//	Light Tower

//###################################################
//###################################################

#include <can_defines.h>
#include <can.h>
#include "HIH613x.h"
#include <Adafruit_ADS1015.h>
//#include "XBee.h"

#include "can_asv3_defines.h"
#include "defines.h"
#include "Arduino.h"
#include "Roboteq.h"
#include "Torqeedo.h"
#include "wind_sensor.h"	// no windsensor used currently

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

uint8_t posb_stat_buf[5] = { 128,256, 0, 0, 0x02};
uint8_t esc1_stat_buf[7] = { 0x2C, 0x1, 0x14, 0x0, 0, 0, 0 }; // 0, 0, 0, 200, 300
uint8_t esc2_stat_buf[7] = { 0x90, 0x1, 0xF4, 0x1, 0, 0, 0 }; // 0, 0, 0, 500, 400
uint8_t esc1_stat_buf2[5] = { 0, 0, 0, 0, 0 }; // 0, 0, 0
uint8_t esc2_stat_buf2[5] = { 0, 0, 0, 0, 0 }; // 0, 0, 0
uint8_t batt1_stat_buf[7] = { 25, 0x2C, 0x1, 0x58, 0x2, 75 , 0}; // 25, 600, 300, 75
uint8_t batt2_stat_buf[7] = { 25, 0x2C, 0x1, 0xBC, 0x2, 50 , 0}; // 25, 700, 300, 50
uint32_t heartbeat_loop;
uint32_t batt_loop;
uint32_t power_loop;
uint32_t sensor_loop;
uint32_t sensor_poll_loop;
uint8_t power_ctr = 0;
uint32_t thruster_loop;
uint32_t thruster_stat_loop;
uint32_t windspeed_loop;	// no windsensor used currently
uint32_t blink_loop;
uint32_t autothruster_loop;
static uint32_t pressure_loop = 0;

uint32_t esc1_loop;
uint32_t esc2_loop;
uint32_t batt1_loop;
uint32_t batt2_loop;
uint32_t controlmode_loop;
bool heartbeat_esc1 = false;
bool heartbeat_esc2 = false;
bool heartbeat_batt1 = false;
bool heartbeat_batt2 = false;

Adafruit_ADS1115 ads(0X48);
uint16_t int_pressure;
uint16_t initial_pressure = 0;

WindSensor windSensor = WindSensor();
uint16_t wind_dir = 0;
uint16_t wind_speed = 0;

HIH613x humidTempSensor(0x27);
static uint32_t Temp_Humid_loop = 0; //250ms loop Publish temp and humidity
uint8_t humid_ctr = 0;
uint8_t int_temperature, int_humidity;

uint8_t leak_signal = 0x02;

//#define _TELE_DED_

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
	windSensor.init();	// windsensor used
  
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
	controlmode_loop = millis();
	power_loop = millis();
	thruster_loop = millis();
	autothruster_loop = millis();
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
      case 'z':
        Serial.println("Kill batt");
        Battery1.onBattery(false);
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
		roboteq1.setMotorSpeed(speed1, 1); //FL
		roboteq1.setMotorSpeed(speed2, 2); //FR
		roboteq2.setMotorSpeed(speed3, 1); //BL
		roboteq2.setMotorSpeed(speed4, 2); //BR
		thruster_loop = millis();
	}
	// Send queries for ESC stats
	if ((millis() - thruster_stat_loop) > THRUSTER_STAT_TIMEOUT)
	{
		roboteq1.requestUpdate();
		roboteq2.requestUpdate();
		thruster_stat_loop = millis();
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
	if ((millis() - batt_loop) > BATT_TIMEOUT)
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
	/*			Sensors					*/
	/************************************/
  if ((millis() - sensor_poll_loop) > SENSOR_POLL_TIMEOUT){
    sensor_poll_loop = millis();
    readPosbStats();
  }
	windSensor.readValues();

	/************************************/
	/*			Light Tower				*/
	/************************************/

	updateLightTower();

	/************************************/
	/*			Miscellaneous			*/
	/************************************/

	resetEscHeartbeat();
	resetBatteryHeartbeat();
	failsafe();

	/************************************/
	/*				CAN					*/
	/************************************/

	// CAN TX
	publishCAN();
	// CAN RX
	checkCANmsg();
}

//==========================================
//				   THRUSTER
//==========================================

// Map from [0 to 6400] to [-1000 to 1000]
void getThrusterSpeed()
{
	speed1 = map(CAN.parseCANFrame(buf, 0, 2), 0, 6400, -1000, 1000);
	speed2 = map(CAN.parseCANFrame(buf, 2, 2), 0, 6400, -1000, 1000);
	speed3 = map(CAN.parseCANFrame(buf, 4, 2), 0, 6400, -1000, 1000);
	speed4 = map(CAN.parseCANFrame(buf, 6, 2), 0, 6400, -1000, 1000);
	
	Serial.print(" 1: ");
	Serial.print(speed1);
	Serial.print(" 2: ");
	Serial.print(speed2);
	Serial.print(" 3: ");
	Serial.print(speed3);
	Serial.print(" 4: ");
	Serial.println(speed4);	
}
void resetThrusterSpeed()
{
	speed1 = 0;
	speed2 = 0;
	speed3 = 0;
	speed4 = 0;
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
		delay(100);
		digitalWrite(LIGHTTOWER_YELLOW, HIGH);
		delay(100);
		digitalWrite(LIGHTTOWER_YELLOW, LOW);
		delay(100);
		digitalWrite(LIGHTTOWER_GREEN, HIGH);
		delay(100);
		digitalWrite(LIGHTTOWER_GREEN, LOW);
		delay(100);
	}
	return;
}

// Set light tower to [colour], and turn off all other lights
void setLightTower(byte colour)
{
	if (colour == LIGHTTOWER_RED)
	{
		digitalWrite(LIGHTTOWER_RED, HIGH);
		//Serial.print("RED ON ");
	}
	else
	{
		digitalWrite(LIGHTTOWER_RED, LOW);
		//Serial.print("RED OFF ");
	}

	if (colour == LIGHTTOWER_YELLOW)
		digitalWrite(LIGHTTOWER_YELLOW, HIGH);
	else
		digitalWrite(LIGHTTOWER_YELLOW, LOW);

	if (colour == LIGHTTOWER_GREEN)
	{
		digitalWrite(LIGHTTOWER_GREEN, HIGH);
		//Serial.print("GREEN ON ");
	}
	else
	{
		digitalWrite(LIGHTTOWER_GREEN, LOW);
		//Serial.print("GREEN OFF ");
	}
}

// GREEN: AUTONOMOUS
// GREEN: (BLINK) STATION KEEP
// YELLOW: MANUAL
// RED: ESTOP
void updateLightTower()
{
	//Serial.print("Light! ");
	if (estop_status)
	{
		//Serial.print("ESTOP!! ");
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
	//Serial.println("");
}

//==========================================
//          POSB_STATS
//==========================================

void readTempHumid()
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
			int_temperature = humidTempSensor.getTemperature() + 0.5;
			int_humidity = humidTempSensor.getHumidity() + 0.5;
#endif
			
			Serial.print("Temp: ");
			Serial.print(posb_stat_buf[0]);
			Serial.print("\t");
			Serial.println(posb_stat_buf[1]);
			humid_ctr = 0;
			break;
		}
		default:
			break;
		}
		Temp_Humid_loop = millis();
}

void readInternalPressure() {
  /*
  VOUT = VS x (0.004 x P - 0.040)�� (Pressure Error x Temp Factor x 0.004 x VS)
  VS = 5.1 �� 0.36 Vdc
  */
  // internal   raw value 9690 = 1010mb = 101kPa  
  ads.set_continuous_conv(1);
  delay(ADS_DELAY);
  uint16_t adc1 = ads.readADC_Continuous();
  int_pressure = (((double)adc1*0.0001875) / (Vref*0.0040) + 10) * 10; 
}

void readLeakSignal(){
  if (digitalRead(4) == HIGH){
    leak_signal = 0x01; 
  }
  else{
    leak_signal = 0x00;
  }
}

void readPosbStats(){
  readTempHumid();
  readInternalPressure();
  readLeakSignal();
}

//==========================================
//          HEARTBEATS
//==========================================

// If stop getting "CONTROLMODE" msg (CAN ID: 103), stop all thrusters
void failsafe()
{
	if ((millis() - controlmode_loop) > CONTROLMODE_TIMEOUT)
	{
#ifndef _TELE_DED_
		control_mode = MANUAL_OCS;
		resetThrusterSpeed();
		Serial.println("*** All thrusters stopped *** No telemetry HB");
#else
		control_mode = AUTONOMOUS;
		Serial.println("thruster not stopped though tele ded.....  QQ");
#endif // !_TELE DED_
		controlmode_loop = millis();
		if ((millis() - autothruster_loop) > INACTIVITY_TIMEOUT)
		{
			resetThrusterSpeed();
			autothruster_loop = millis();
		}
	}
}
void resetBatteryHeartbeat()
{
	// Turn off heartbeat if no batt response for 1s
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
}

void resetEscHeartbeat()
{
	// Turn off heartbeat if no esc response for 1s
	if ((millis() - esc1_loop) > INACTIVITY_TIMEOUT)
	{
		heartbeat_esc1 = false;
    Serial.println("Esc1 dead ....................................");
		esc1_loop = millis();
	}
	if ((millis() - esc2_loop) > INACTIVITY_TIMEOUT)
	{
    Serial.println("Esc2 dead ....................................");
		heartbeat_esc2 = false;
		esc2_loop = millis();
	}
}

//==========================================
//					CAN
//==========================================

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

  if ((millis() - sensor_loop) > SENSOR_TIMEOUT){
    sensor_loop = millis();
    publishCAN_posbstats();
  }
}

void publishCAN_posbstats()
{
  CAN.setupCANFrame(posb_stat_buf, 0, 1, int_temperature);
  CAN.setupCANFrame(posb_stat_buf, 1, 1, int_humidity);
  //CAN.setupCANFrame(posb_stat_buf, 2, 2, int_pressure); disabiling pressure temporarily
  CAN.setupCANFrame(posb_stat_buf, 4, 1, leak_signal);
  #ifdef _TEST_
    Serial.print("temperature: ");
    Serial.print(int_temperature);
    Serial.print(" humidity: ");
    Serial.print(int_humidity);
    Serial.print(" pressure: ");
    Serial.println(int_pressure);
  #endif
	CAN.sendMsgBuf(CAN_POSB_STATS, 0, 5, posb_stat_buf);
}
void publishCAN_heartbeat()
{
	buf[0] = HEARTBEAT_POSB;
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
  if (heartbeat_batt1) {
    buf[0] = HEARTBEAT_BATT1;
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
  }
  if (heartbeat_batt2) {
    buf[0] = HEARTBEAT_BATT2;
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
  }
  if (heartbeat_esc1) {
    Serial.println("ESC1 heartbeat sent");
    buf[0] = HEARTBEAT_ESC1;
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
    CAN.sendMsgBuf(25, 0, 1, buf);
  }
  if (heartbeat_esc2) {
    Serial.println("ESC2 heartbeat sent");
    buf[0] = HEARTBEAT_ESC2;
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
    CAN.sendMsgBuf(25, 0, 1, buf);
  }
	Serial.println("HEARTBEAT!");
}
void publishCAN_windspeed()
{
	wind_dir = windSensor.getDirection();
	wind_speed = windSensor.getWindSpeed();
	#ifdef _TEST_
	  Serial.print(" WIND: ");
	  Serial.print(wind_dir);
	  Serial.print(" ");
	  Serial.print(wind_speed);
	#endif
	CAN.setupCANFrame(buf, 0, 2, wind_dir);
	CAN.setupCANFrame(buf, 2, 2, wind_speed);
	CAN.sendMsgBuf(CAN_WIND_SENSOR_STATS, 0, 4, buf);
}

void publishCAN_esc1_stats()
{

	RoboteqStats esc1_stats = roboteq1.getRoboteqStats();
//	Serial.print("Current(A): ");
//	Serial.print(esc1_stats.motor_current1);
//	Serial.print("\t");
//	Serial.print(esc1_stats.motor_current2);
//	Serial.print(" Motor flags: ");
//	Serial.print(esc1_stats.motor_status_flags1, BIN);
//	Serial.print(" ");
//	Serial.print(esc1_stats.motor_status_flags2, BIN);
//	Serial.print(" Fault flags: ");
//	Serial.println(esc1_stats.fault_flags, BIN);
	CAN.setupCANFrame(esc1_stat_buf, 0, 2, esc1_stats.motor_current1);
	CAN.setupCANFrame(esc1_stat_buf, 2, 2, esc1_stats.motor_current2);
	CAN.setupCANFrame(esc1_stat_buf, 4, 1, esc1_stats.motor_status_flags1);
	CAN.setupCANFrame(esc1_stat_buf, 5, 1, esc1_stats.motor_status_flags2);
	CAN.setupCANFrame(esc1_stat_buf, 6, 1, esc1_stats.fault_flags);
	CAN.sendMsgBuf(CAN_ESC1_MOTOR_STATS, 0, 7, esc1_stat_buf);
  CAN.setupCANFrame(esc1_stat_buf2, 0, 2, esc1_stats.motor_power1);
  CAN.setupCANFrame(esc1_stat_buf2, 2, 2, esc1_stats.motor_power2);
  CAN.setupCANFrame(esc1_stat_buf2, 4, 1, esc1_stats.mcu_temp);
  CAN.sendMsgBuf(CAN_ESC1_MOTOR_STATS2, 0, 5, esc1_stat_buf2);
}
void publishCAN_esc2_stats()
{
	RoboteqStats esc2_stats = roboteq2.getRoboteqStats();
	CAN.setupCANFrame(esc2_stat_buf, 0, 2, esc2_stats.motor_current1);
	CAN.setupCANFrame(esc2_stat_buf, 2, 2, esc2_stats.motor_current2);
	CAN.setupCANFrame(esc2_stat_buf, 4, 1, esc2_stats.motor_status_flags1);
	CAN.setupCANFrame(esc2_stat_buf, 5, 1, esc2_stats.motor_status_flags2);
	CAN.setupCANFrame(esc2_stat_buf, 6, 1, esc2_stats.fault_flags);
	CAN.sendMsgBuf(CAN_ESC2_MOTOR_STATS, 0, 7, esc2_stat_buf);
  CAN.setupCANFrame(esc2_stat_buf2, 0, 2, esc2_stats.motor_power1);
  CAN.setupCANFrame(esc2_stat_buf2, 2, 2, esc2_stats.motor_power2);
  CAN.setupCANFrame(esc2_stat_buf2, 4, 1, esc2_stats.mcu_temp);
  CAN.sendMsgBuf(CAN_ESC2_MOTOR_STATS2, 0, 5, esc2_stat_buf2);
}
void publishCAN_batt1_stats()
{
#ifdef _TEST_
  Serial.println("battery 1");
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
  if (heartbeat_batt1) {
  	CAN.setupCANFrame(batt1_stat_buf, 0, 1, Battery1.getCapacity());
  	CAN.setupCANFrame(batt1_stat_buf, 1, 2, Battery1.getVoltage()*100);
  	CAN.setupCANFrame(batt1_stat_buf, 3, 2, 0 - (int16_t)Battery1.getCurrent()*100);
  	CAN.setupCANFrame(batt1_stat_buf, 5, 2, Battery1.getTemperature());
  	CAN.sendMsgBuf(CAN_BATT1_STATS, 0, 7, batt1_stat_buf);
  }
}
void publishCAN_batt2_stats()
{
  #ifdef _TEST_
  Serial.println("battery 2");
  Serial.print("Capacity(%): ");
  Serial.print(Battery1.getCapacity());
  Serial.print(" Voltage(V): ");
  Serial.print(Battery1.getVoltage());
  Serial.print(" Current(A): ");
  Serial.print(Battery1.getCurrent());
  Serial.print(" Temp(C): ");
  Serial.println(Battery1.getTemperature());
#endif
  if (heartbeat_batt2) {
  	CAN.setupCANFrame(batt2_stat_buf, 0, 1, Battery2.getCapacity());
  	CAN.setupCANFrame(batt2_stat_buf, 1, 2, Battery2.getVoltage()*100);
  	CAN.setupCANFrame(batt2_stat_buf, 3, 2, 0 - (int16_t)Battery2.getCurrent()*100);
  	CAN.setupCANFrame(batt2_stat_buf, 5, 2, Battery2.getTemperature());
  	CAN.sendMsgBuf(CAN_BATT2_STATS, 0, 7, batt2_stat_buf);
  }
}
void checkCANmsg() {
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		switch (id) {
#ifndef _TEST_
		case CAN_THRUSTER:
			Serial.print("[");
			Serial.print(control_mode);
			Serial.print("]");
			if ((control_mode == AUTONOMOUS) || (control_mode == STATION_KEEP))
			{
				Serial.print(" AUTO: ");
				getThrusterSpeed();
				autothruster_loop = millis();
			}
			break;
		case CAN_MANUAL_THRUSTER:
			Serial.print("[");
			Serial.print(control_mode);
			Serial.print("]");
			if ((control_mode == MANUAL_OCS) || (control_mode == MANUAL_RC))
			{
				//Serial.print(" MANUAL: ");
				getThrusterSpeed();
			}
			break;
#endif
		case CAN_CONTROL_LINK:
			control_mode = CAN.parseCANFrame(buf, 0, 1);
			controlmode_loop = millis();
			break;
		case CAN_E_STOP:
			estop_status = CAN.parseCANFrame(buf, 0, 1);
			break;
		case ROBOTEQ_CAN1_REPLY_INDEX:
			roboteq1.readRoboteqReply(id, len, buf);
			heartbeat_esc1 = true;
			esc1_loop = millis();
      Serial.println("esc 1 read data\n");
			break;
		case ROBOTEQ_CAN2_REPLY_INDEX:
			roboteq2.readRoboteqReply(id, len, buf);
			heartbeat_esc2 = true;
			esc2_loop = millis(); 
      Serial.println("esc 2 read data\n");
			break;
    case CAN_BATT_CTRL:
      batt_ctrl = CAN.parseCANFrame(buf, 0, 1);
      if (batt_ctrl == 1) Battery1.onBattery(false);
		default:
			Serial.println("Others");
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
