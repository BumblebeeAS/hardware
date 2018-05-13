//###################################################
//###################################################
//
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####    
//
//
//Sensor and Telemetry for BBAUV 3.99
//Firmware Version :             v1.0
////
// Written by Chia Che
// Change log v0.0:
//
//###################################################
//###################################################

#include <Wire.h>
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ADS1015.h>
#include <HIH613x.h>
#include "LEDS.h"
#include "LCD_Driver.h"
#include "define.h"
#include <can_defines.h>
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include "can_auv_define.h"

// CAN variable
MCP_CAN CAN(CAN_Chip_Select);
uint32_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

//Screen variables
LCD screen = LCD(SCREEN_CS, SCREEN_RESET);  
static uint16_t internalStats[INT_STAT_COUNT] = { 0 };
static uint16_t powerStats[POWER_STAT_COUNT] = { 0 };
static uint32_t heartbeat_timeout[HB_COUNT] = { 0 };
static uint32_t loopTime = 0;

//Sensor variables
Adafruit_ADS1115 ads(ADS_ADDR);
HIH613x humid(HUMIDITY_ADDR);
uint8_t humidity = 0;
uint8_t IntPressure = 0;
uint8_t temperature = 0;
uint16_t ExtPressure = 0;
uint8_t InitialP = 0;
uint16_t rawExtPressure = 0;
static uint32_t humidloop = 0;
bool readHumid = false;
static uint32_t pressure_loop = 0;
static uint32_t filter_loop = 0;

//LED
LEDS led(RED, GREEN, BLUE);
uint8_t lightColour = 9;	// 9 is off

//Others
uint8_t CPU_CoreTemp = 0;
bool sonar = false;
static uint32_t pmb1_timeout = 0;
static uint32_t	pmb2_timeout = 0;
static uint32_t sbc_timeout = 0;
static uint32_t dna_timeout = 0;
static uint32_t heartbeat_loop = 0;
static uint32_t stats_loop = 0;
static uint16_t dna_pressure = 0;

static uint32_t testing_time = 0;

void setup()
{
	pinMode(SCREEN_CS, OUTPUT);				//CS screen
	digitalWrite(SCREEN_CS, HIGH);
	pinMode(CAN_Chip_Select, OUTPUT);		//CS CAN
	digitalWrite(CAN_Chip_Select, HIGH);

	Serial.begin(115200);
	Serial.println("Hi, I'm STB!");
	
	//CAN init
	CAN_init();
	Serial.println("CAN OK");
	CANSetMask();

	//Screen init
	screen.screen_init();
	Serial.println("Screen Ok");
	screen_prepare();

	//Sensor init
	Wire.begin();
	Serial.println("Sensors OK");
	InitialP = readInternalPressure();

	//led init
	led_init();
	Serial.println("LED OK");
	
	for (int i = 0; i < HB_COUNT; i++) {
		heartbeat_timeout[i] = millis();
	}
}

void loop()
{
	reset_stats();
	update_ST_stats();

	if ((millis() - loopTime) > SCREEN_LOOP) {	//	1000ms
		screen_update();
		update_heartbeat();
		loopTime = millis();
	}
	
	if (millis() - filter_loop > LPF_LOOP) {	//	25ms
		externalPressureLPF();
		filter_loop = millis();
	}

	checkCANmsg();

	publishCAN();	
}

//===========================================
//
//				CAN FUNCTIONS
//
//===========================================

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

void CANSetMask() {
	/*
	Truth table
	mask	filter	  id bit	reject
	0		  X			X		  no
	1		  0			0		  no
	1		  0		    1		  yes
	1		  1		    0		  yes
	1		  1			1		  no

	Mask 0 connects to filt 0,1
	Mask 1 connects to filt 2,3,4,5

	Mask decide which bit to check
	Filt decide which bit to accept
	*/

	CAN.init_Mask(0, 0, 0xE);	//	check	111X
	CAN.init_Mask(1, 0, 0xF);	//	check	all bit

	CAN.init_Filt(0, 0, 0xA);	// let	101X pass (10,11)
	CAN.init_Filt(1, 0, 0xC);	// let	110X pass (12,13)

	CAN.init_Filt(2, 0, 0x3);	// let	0011 pass (3)
	CAN.init_Filt(3, 0, 0x4);	// let	0100 pass (4)
	CAN.init_Filt(4, 0, 0x9);	// let	1001 pass (9)
	CAN.init_Filt(5, 0, 0xE);	// let	1110 pass (14)
	
}

/*			Receive these CAN ID
3:	Heartbeat: SBC, SBC_CAN, PCB, TB, ST, MANI, PMB1, PMB2
4:	Sonar trigger
9:	LED
10:	PMB1 stat1
11:	PMB1 stat2
12:	PMB2 stat1
13:	PMB2 stat2
14:	CPU Temp
*/

void checkCANmsg() {
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		switch (CAN.getCanId()) {
		case CAN_heartbeat: 
		{
			uint32_t device = CAN.parseCANFrame(buf, 0, 1);
			heartbeat_timeout[device] = millis();
			break;
		}
		case CAN_SONAR: 
		{
			uint8_t temp = CAN.parseCANFrame(buf, 0, 1);
			temp == 1 ? sonar = true : sonar = false;
			break;
		}
		case CAN_LED:
			lightColour = CAN.parseCANFrame(buf, 0, 1);
			led.colour(lightColour);
			break;
		case CAN_DNA_Stats:
			internalStats[DNA_PRESS] = CAN.parseCANFrame(buf, 2, 1);
			sbc_timeout = millis();
			break;
		case CAN_PMB1_stats:
			powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 0, 2);
			powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 2, 2);
			pmb1_timeout = millis();
			break;
		case CAN_PMB1_stats2:
			internalStats[PMB1_PRESS] = CAN.parseCANFrame(buf, 4, 1);
			internalStats[PMB1_TEMP] = CAN.parseCANFrame(buf, 3, 1);
			powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 2, 1);
			pmb1_timeout = millis();
			break;
		case CAN_PMB2_stats:
			powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 0, 2);
			powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 2, 2);
			pmb2_timeout = millis();
			break;
		case CAN_PMB2_stats2:
			internalStats[PMB2_PRESS] = CAN.parseCANFrame(buf, 4, 1);
			internalStats[PMB2_TEMP] = CAN.parseCANFrame(buf, 3, 1);
			powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 2, 1);
			pmb2_timeout = millis();
			break;
		case CAN_CPU:
		{
			uint8_t temp[5] = { 0 };
			for (int i = 0; i < 5; i++) {
				temp[i] = CAN.parseCANFrame(buf, i, 1);
			}
			uint8_t max = temp[0];
			for (int i = 1; i < 5; i++) {
				if (temp[i] >= max) {
					max = temp[i];
				}
			}
			CPU_CoreTemp = max;
			sbc_timeout = millis();
			break;
		}
		default:
			break;
		}
		CAN.clearMsg();
	}
}

//publish raw pressure, heartbeat and stats to CAN bus
void publishCAN()
{
	//publish raw external pressure reading every 50ms
	if (millis() - pressure_loop > 50) {
		publishCAN_pressure();
		pressure_loop = millis();
	}
	
	//publish heartbeat every 500ms
	if (millis() - heartbeat_loop > 500) {
		publishCAN_heartbeat(HEARTBEAT_ST);
		heartbeat_loop = millis();
	}

	//publish ST stats every 1000ms
	if (millis() - stats_loop > 1000) {
		publishST_stats();
		stats_loop = millis();
	}
}

void publishCAN_heartbeat(int device_id)
{
	id = CAN_heartbeat;
	len = 1;
	buf[0] = device_id;
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

void publishCAN_pressure() {
	CAN.setupCANFrame(buf, 0, 2, rawExtPressure);
	CAN.sendMsgBuf(CAN_pressure, 0, 2, buf);
}

void publishST_stats() {
	CAN.setupCANFrame(buf, 0, 1, temperature);
	CAN.setupCANFrame(buf, 1, 1, humidity);
	CAN.setupCANFrame(buf, 2, 1, IntPressure);
	leak() ? CAN.setupCANFrame(buf, 3, 1, 1) : CAN.setupCANFrame(buf, 3, 1, 0);
	CAN.sendMsgBuf(CAN_ST_stats, 0, 4, buf);
}


//==========================================
//
//				LCD FUNCTIONS
//
//==========================================

void screen_prepare() {
	screen.set_cursor(0 + OFFSET, 0);
	screen.write_string("Ext press:");
	screen.write_string("Int press:");
	screen.write_string("PMB1 press:");
	screen.write_string("PMB2 press:");
	screen.write_string("PMB1 temp:");
	screen.write_string("PMB2 temp:");
	screen.write_string("CPU temp:");
	screen.write_string("Humidity:");
	screen.write_string("ST temp:");
	screen.write_string("DNA press: ");
	screen.write_string("SBC OK:");
	screen.write_string("SBC-CAN OK:");
	screen.write_string("PCB OK:");
	
	screen.set_cursor(400 + OFFSET, 0);
	screen.write_string("Batt1 capacity:");
	screen.write_string("Batt2 capacity:");
	screen.write_string("Batt1 current:");
	screen.write_string("Batt2 current:");
	screen.write_string("Batt1 voltage:");
	screen.write_string("Batt2 voltage:");
	screen.write_string("Thruster OK:");
	screen.write_string("Manipulator OK:");
	screen.write_string("PMB1 OK:");
	screen.write_string("PMB2 OK:");
}

void screen_update() {
	// row height 35,     increment_row()
	screen.set_cursor(200 + OFFSET, 0);
	for (int i = 0; i < INT_STAT_COUNT; i++)
	{
		screen.write_value_int(internalStats[i]);
	}

	screen.set_cursor(645 + OFFSET, 0);
	for (int i = 0; i < POWER_STAT_COUNT; i++)
	{
		if (i > BATT2_CAPACITY) {
			screen.write_value_with_dp(powerStats[i], 1);
		}
		else {
			screen.write_value_int(powerStats[i]);
		}
	}
}

void update_heartbeat()
{
	// row height 35,     increment_row()
	int i;
	screen.set_cursor(200 + OFFSET, 350);
	for (i = 1; i < 4; i++) {
		if (i != HEARTBEAT_ST) // Skip ST HB
		{
			if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
				screen.write_value_string("NO");
			}
			else
				screen.write_value_string("YES");
		}
	}

	screen.set_cursor(645 + OFFSET, 210);
	for (; i < 9; i++) {
		if (i != HEARTBEAT_ST) { // Skip ST HB
			if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
				screen.write_value_string("NO");
			}
			else
				screen.write_value_string("YES");
		}
	}
}

//reset pmb1 pmb2 and sbc stats 
void reset_stats()
{
	reset_pmb1_stat();
	reset_pmb2_stat();
	reset_sbc_stat();
}

void reset_pmb1_stat() {
	if ((millis() - pmb1_timeout) > STAT_TIMEOUT) {
		internalStats[PMB1_PRESS] = 0xFFFF;
		internalStats[PMB1_TEMP] = 0xFFFF;
		powerStats[BATT1_CAPACITY] = 0xFFFF;
		powerStats[BATT1_CURRENT] = 0xFFFF;
		powerStats[BATT1_VOLTAGE] = 0xFFFF;
		pmb1_timeout = millis();
	}
}

void reset_pmb2_stat() {
	if ((millis() - pmb2_timeout) > STAT_TIMEOUT) {
		internalStats[PMB2_PRESS] = 0xFFFF;
		internalStats[PMB2_TEMP] = 0xFFFF;
		powerStats[BATT2_CAPACITY] = 0xFFFF;
		powerStats[BATT2_CURRENT] = 0xFFFF;
		powerStats[BATT2_VOLTAGE] = 0xFFFF;
		pmb2_timeout = millis();
	}
}

void reset_sbc_stat() {
	if ((millis() - sbc_timeout) > STAT_TIMEOUT) {
		internalStats[CPU_TEMP] = 0xFFFF;
		internalStats[DNA_PRESS] = 0xFFFF;
		sbc_timeout = millis();
	}
}

//read Temperature, Humidity, External and Internal pressure
// and assign them to array for update
void update_ST_stats() {
	readTempHumididty();
	rawExtPressure = readExternalPressure();
	internalStats[EXT_PRESS] = ExtPressure;
	IntPressure = readInternalPressure();
	internalStats[INT_PRESS] = readInternalPressure();
	internalStats[HUMIDITY] = humidity;
	internalStats[ST_TEMP] = temperature;
}

//==========================================
//
//				Sensor Functions
//
//==========================================

//Return Internal Pressure 
byte readInternalPressure() {
	/*
	VOUT = VS x (0.004 x P - 0.040)กำ (Pressure Error x Temp Factor x 0.004 x VS)
	VS = 5.1 กำ 0.36 Vdc
	*/
	// internal   raw value 9690 = 1010mb = 101kPa
	ads.set_continuous_conv(1);
	delay(ADS_DELAY);		
	uint16_t adc1 = ads.readADC_Continuous();

	return (((double)adc1*0.0001875) / (Vref*0.0040) + 10);
}

//Updates ExtPressure and return raw 16bit External Pressure reading
uint16_t readExternalPressure() {
	//output 4-20mA  range upto 50psi
	// ==> 1-5V as shunt resistor is 250.0ohm
	ads.set_continuous_conv(0);
	delay(ADS_DELAY);
	uint16_t adc0 = ads.readADC_Continuous();
	
	//psi to pascal
	double temp_ext = 0.0;
	temp_ext = 6895 * ((15.0 * (adc0 / 10684.0) - 7.498596031));	//Convert to Pascal
	temp_ext = temp_ext + 99973.98; //plus 1 ATM
	ExtPressure = (uint16_t)(temp_ext / 1000);
	if (ExtPressure < 80 || ExtPressure > 350) {
		ExtPressure = 0xFFFF;
	}
	
	return adc0;
}

//Filter External Pressure with LPF
void externalPressureLPF() {
	//LPF filter for ext pressure sensor
	uint16_t temp = readExternalPressure();
	if (temp != 0)
	{
	rawExtPressure = rawExtPressure + LPF_CONSTANT * (float)(temp - rawExtPressure);
	}

}

//Updates Temperature and Humidity
void readTempHumididty() {
	// reading temp or humid takes 36.65ms, 2 takes 74ms
	if (millis() - humidloop > 100) {
		if (!readHumid) {
			humid.measurementRequest();
			readHumid = true;
		}
		else {
			humid.dataFetch();
			humidity = humid.getHumidity() + 0.5;
			temperature = humid.getTemperature() + 0.5;
			readHumid = false;
		}
		humidloop = millis();
	}
}

//Blinks through all colour
void led_init() {
	for (int i = 0; i < 11; i++) {
		led.colour(i);
		delay(200);
	}
	led.colour(lightColour);
}

//Return bool to indicate whether is it leaking
//Blinks led if it is leaking
bool leak() {
	bool leaking = false;
	if ((InitialP - IntPressure > 10) || humidity > 85) {
		leaking = true;
	}
	if (leaking) {
		//led.colour(8);	//	8 for white
		led.blink(1, 8, 300);		// red white 500ms
	}
	return leaking;
}

void sonar_init() {
	pinMode(SONAR_IN, OUTPUT);
	pinMode(SONAR_OUT, OUTPUT);
	digitalWrite(SONAR_OUT, LOW);

	sonar ? digitalWrite(SONAR_IN, HIGH) : digitalWrite(SONAR_IN, LOW);
}

//Updates hardware trigger of Sonar
void sonar_update() {
	digitalWrite(SONAR_OUT, LOW);
	sonar ? digitalWrite(SONAR_IN, HIGH) : digitalWrite(SONAR_IN, LOW);
}