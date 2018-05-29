#include "Arduino.h"
#include "pmb_define.h"
#include <Wire.h>
#include <EEPROM.h>
#include "INA233.h"
#include <SSD1306_text.h>
#include <can.h>
#include "can_auv_define.h"			
#include <TempAD7414.h>
#include <Adafruit_ADS1015.h>

INA233 monIc(MAX_CURR, SHUNT_RES);
SSD1306_text display(PIN_OLED_RESET);
MCP_CAN CAN(CAN_CS_PIN);

uint8_t BATT_STATE = 0;
uint16_t voltageArray[ARR_SIZE] = { 0 };
uint8_t voltageIdx = 0;
uint16_t voltage;
int16_t shuntVolt;
int16_t currentArray[ARR_SIZE] = { 0 };
uint8_t currentIdx = 0;
int16_t currByte;
float current;
uint32_t capacityLeft = MAX_CAPACITY;


uint32_t voltTimer = 0;
uint32_t currTimer = 0;
uint32_t printTimer = 0;
uint32_t oledTimer = 0;
uint32_t onTimer;
uint32_t tempTimer = 0;
uint32_t pressureTimer = 0;
uint32_t now;

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };
uint32_t CanHeartbeatLoop = 0;
uint32_t CanStatusLoop = 0;

float board_temperature = 0;
TempAD7414 TempSensor(CONFIG_TEMP_SENS, ADDR_TEMP_SENS);

float board_pressure = 0;
Adafruit_ADS1115 ads;
int16_t adc1;

void setup()
{
	/* MCU Initialisation */
	pinMode(PIN_PMOS, OUTPUT);
	pinMode(PIN_RELAY, OUTPUT);
	pinMode(PIN_OFF, INPUT);
	digitalWrite(PIN_RELAY, LOW);
	digitalWrite(PIN_PMOS, LOW);

	Serial.begin(115200);
	/* End of MCU Initialisation */

	/* CAN Initialisation*/

	CAN_init();
	// CAN Masking
	

	/* End of CAN Initialisation*/

	/* Peripherals Initialisation */
	Wire.begin();

	/* Display Init */
	Serial.println("Initialising Display");
	display.init();

	/* Temp Sensor Init*/
	Serial.println("Initialising Temp Sensor");
	TempSensor.initTempAD7414();

	/* Pressure Sensor Init*/
	Serial.println("Initialising Pressure Sensor");
	ads.begin();

	/* INA233 Init */
	Serial.println("Initialising Current IC");
	monIc.init();
	Serial.println("Calibration complete");

	Serial.println("Averaging voltage");
	for (int i = 0; i < ARR_SIZE; i++) {
		voltage = rollVoltAvg(monIc.readVoltage());
		delay(20);
	}
	loadDataFromEeprom();
	delay(100);
	digitalWrite(PIN_PMOS, HIGH);
	Serial.println("pMOS on!");

	/* Clear and update display with initial data */
	display.clear();
	updateDisplay();
}

void loop()
{
	//Check for soft off button
	if (digitalRead(PIN_OFF)) {
		digitalWrite(PIN_PMOS, LOW);
		displayOffMessage();
		saveDataToEeprom();
		digitalWrite(PIN_RELAY, HIGH);
		/* PMB is turned off */
	}
	now = millis();
	// POPB Heartbeat
	if ((millis() - CanHeartbeatLoop) > CAN_HEARTBEAT_INTERVAL) {
		publishCanHB();
		CanHeartbeatLoop = millis();
	}

	now = millis();
	if ((millis() - CanStatusLoop) > CAN_STATUS_INTERVAL) {
		publishCanStatus();
		CanStatusLoop = millis();
	}

	now = millis();
	if (now - voltTimer > VOLT_INTERVAL) {
		voltTimer = now;
		voltage = rollVoltAvg(monIc.readVoltage());
	}

	now = millis();
	if (now - currTimer > CURR_INTERVAL) {
		currTimer = now;
		shuntVolt = monIc.readShuntVoltage();
		currByte = rollCurrAvg(monIc.readCurrent());
		current = monIc.floatCurrent(currByte);
		capacityLeft = capacityLeft - (int32_t)(1000.0*current); //*1000 = convert to milliA. *0.01 is 10ms per reading, scale 100
		if (capacityLeft > MAX_CAPACITY) {
			capacityLeft = MAX_CAPACITY;
			BATT_STATE = BATT_FULL;
		}
		else if (current < 0) {
			BATT_STATE = BATT_CHARGING;
		}
		else {
			BATT_STATE = BATT_DISCHARGING;
		}
	}

	now = millis();
	if (now - tempTimer > TEMP_INTERVAL) {
		tempTimer = now;
		board_temperature = TempSensor.getTemp();
	}

	now = millis();
	if (now - pressureTimer > PRESSURE_INTERVAL) {
		pressureTimer = now;
		adc1 = ads.readADC_SingleEnded(1);
		board_pressure = ((float)adc1*0.0001875) / (INTPRES_REF * 0.0040) + 10;
	}

	now = millis();
	if (now - printTimer > PRINT_INTERVAL) {
		printTimer = now;
		Serial.print("V: ");
		Serial.print(voltage / 100.0);
		Serial.print(" | I: ");
		Serial.print(current, 3);
		Serial.print(" | Batt%: ");
		Serial.println(capacityLeft*100.0 / MAX_CAPACITY, 2);
	}

	now = millis();
	if (now - oledTimer > OLED_INTERVAL) {
		oledTimer = now;
		updateDisplay();
	}
}

//Methods

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

void publishCanStatus() {
	uint8_t PMB_stats1[8] = { 0 };
	uint8_t PMB_stats2[8] = { 0 };
	CAN.setupCANFrame(PMB_stats1, 0, 2, (int)(current*10));
	CAN.setupCANFrame(PMB_stats1, 2, 2, voltage/10);

	CAN.setupCANFrame(PMB_stats2, 0, 2, 0);
	CAN.setupCANFrame(PMB_stats2, 2, 1,	capacityLeft*100/MAX_CAPACITY);
	CAN.setupCANFrame(PMB_stats2, 3, 1, uint8_t(board_temperature));
	CAN.setupCANFrame(PMB_stats2, 4, 1, uint8_t(board_pressure));

	CAN.sendMsgBuf(PMB_STATS_ID, 0, 4, PMB_stats1);
	CAN.sendMsgBuf(PMB_STATS2_ID, 0, 5, PMB_stats2);
}

void publishCanHB() {
	uint8_t HB[1] = { PMB_HEARTBEAT_ID }; //HEARTBEAT_PMB
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, HB);
}

void saveDataToEeprom() {
	Serial.println("Saving data to EEPROM, wait for 3s for volt to stabilise");
	delay(3000);
	for (int i = 0; i < ARR_SIZE; i++) {
		voltage = rollVoltAvg(monIc.readVoltage());
		delay(20);
	}
	EEPROM.put(ADDR_CAPACITY, capacityLeft);
	EEPROM.put(ADDR_VOLTAGE, voltage);
	delay(200); //delay as eeprom takes 3.3ms to write 1 byte
}

void loadDataFromEeprom() {
	Serial.println("Checking EEPROM data");
	uint16_t voltageEeprom;
	uint32_t capacityEeprom;
	EEPROM.get(ADDR_VOLTAGE, voltageEeprom);
	EEPROM.get(ADDR_CAPACITY, capacityEeprom);
	delay(200);
	uint16_t temp = voltage - voltageEeprom;
	Serial.print("old: ");
	Serial.print(voltageEeprom);
	Serial.print(" | new: ");
	Serial.println(voltage);
	if (abs(temp) > VOLTAGE_DIFF) { // Difference too large. probably battery recharged
									//Read capacity using battery voltage :: assume max for now
		capacityLeft = MAX_CAPACITY;
		Serial.println("New capacity");
	}
	else {
		//Read capacity from EEPROM
		capacityLeft = capacityEeprom;
		Serial.println("Old capacity");
	}
}

String getBattState(uint8_t BATT_STATE) {
	if (BATT_STATE == 0)
		return "DISCHARGE";
	else if (BATT_STATE == 1)
		return "CHARGE";
	else
		return "IDLE";
}

uint16_t rollVoltAvg(uint16_t newVolt) {
	voltageArray[voltageIdx++] = newVolt;
	voltageIdx = (voltageIdx == ARR_SIZE) ? 0 : voltageIdx;
	uint32_t sum = 0;
	for (int i = 0; i < ARR_SIZE; i++) {
		sum += voltageArray[i];
	}
	return sum / ARR_SIZE;
}

int16_t rollCurrAvg(int16_t newCurr) {
	currentArray[currentIdx++] = newCurr;
	currentIdx = (currentIdx == ARR_SIZE) ? 0 : currentIdx;
	int32_t sum = 0;
	for (int i = 0; i < ARR_SIZE; i++) {
		sum += currentArray[i];
	}
	return sum / ARR_SIZE;
}

void updateDisplay() {
	display.setTextSize(1, 1);
	display.setCursor(0, 0);
	display.write("Battery PMB ");
	display.print(PMB_NO);
	display.setCursor(1, 0);
	display.write("Batt %:      ");
	display.setCursor(1, 48);
	display.write("       ");
	display.setCursor(1, 48);
	display.print(capacityLeft*100.0 / MAX_CAPACITY);
	display.setCursor(2, 0);
	display.write("Batt Volt: ");
	display.setCursor(2, 68);
	display.write("         ");
	display.setCursor(2, 68);
	display.print(voltage/100.0);
	display.setCursor(3, 0);
	display.write("Current: ");
	display.setCursor(3, 68);
	display.write("         ");
	display.setCursor(3, 68);
	display.print(current);
	display.setCursor(4, 0);
	display.write("State: ");
	display.setCursor(4, 38);
	display.write("              ");
	display.setCursor(4, 38);
	display.print((getBattState((current>0)?0:1)));
	display.setCursor(5, 0);
	display.write("Pod Temp: ");
	display.print(board_temperature);
	display.setCursor(6, 0);
	display.write("Pod Pres: ");
	display.print(board_pressure);
	display.setCursor(7, 0);
	display.write("Low Batt: ");
	//(batt_low) ? display.print("YES") : display.print("NO");
}

void displayOffMessage() {
	display.clear();
	display.setTextSize(1, 1);
	display.setCursor(0, 0);
	display.write("Battery PMB ");
	display.print(PMB_NO);
	display.setCursor(4, 0);
	display.write("Turning off...");
}

//void readVoltage(){
//	uint8_t cmd = 0x88;
//	uint8_t numByte = 2;
//	read_I2C(cmd, numByte);
//	voltage = (( buff[0] << 8) | buff[1]) >> 3; //increments of 10mV (Resolution is 1.25mV/bit)
//	clearBuffer();
//}
//
//void readShuntVoltage(){
//	uint8_t cmd = 0xD1;
//	uint8_t numByte = 2;
//	read_I2C(cmd, numByte);
//	shuntVolt = ((buff[0] << 8) | buff[1]) >> 2; //increments of 10uV (Resolution is 2.5uV/bit)
//	clearBuffer();
//}
//
//void readCurrent(){
//	uint8_t cmd = 0x89;
//	uint8_t numByte = 2;
//	read_I2C(cmd, numByte);
//	int16_t curr = ((buff[0] << 8) | buff[1]);
//	shuntCurrent = currLSB * curr / 2000000.0; //Resolution 4.5mA
//	rollCurrAvg(curr);
//	clearBuffer();
//}
//
//void clearEnergyAcc() {
//	Wire.beginTransmission(addr); // start transmission
//	Wire.write(0xD6); // send command
//	Wire.endTransmission();
//}
//
//void readEnergy(){
//	uint8_t cmd = 0x86;
//	uint8_t numByte = 6 + 1; //1 more for size
//	read_I2C(cmd, numByte);
//	Serial.print(buff[0], HEX);
//	Serial.print(buff[1], HEX);
//	Serial.print(buff[2], HEX);
//	Serial.print(buff[3], HEX);
//	Serial.print(buff[4], HEX);
//	Serial.println(buff[5], HEX);
//	clearEnergyAcc();
//	clearBuffer();
//}
