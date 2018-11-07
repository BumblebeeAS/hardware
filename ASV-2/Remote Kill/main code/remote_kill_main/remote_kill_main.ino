#include <SoftwareSerial.h>
#include "can_asv_defines.h"

#define SERIAL_BAUD_RATE 115200
#define N2420_BAUD_RATE 9600
#define START_BYTE 0xFE
#define NO_KILL_BYTE 0x15
#define KILL_BYTE 0x44

#define REMOTE_KILL 3
#define LOW_BATT A1
#define BATTERY_VOLTAGE A2

#define SEND_POKB_TIMEOUT 500

// Time Counter Variables
uint32_t sendPOKBTime = 0;

// Radio Variables
int canID = CAN_POKB_BUS_stats;
int dataLength = 1;
uint8_t data[1] = { NO_KILL_BYTE };

SoftwareSerial radioSerial = SoftwareSerial(6, 7); // RX: 6, TX: 7

void setup() {
	Serial.begin(SERIAL_BAUD_RATE);
	radioSerial.begin(N2420_BAUD_RATE);

	pinMode(REMOTE_KILL, INPUT);
	pinMode(BATTERY_VOLTAGE, INPUT);
	pinMode(LOW_BATT, OUTPUT);
}

void loop() {

	if ((millis() - sendPOKBTime) > SEND_POKB_TIMEOUT) {
		if (digitalRead(REMOTE_KILL) == HIGH) {
			data[0] = NO_KILL_BYTE;
			Serial.println("No Kill.");
		}
		else {
			data[0] = KILL_BYTE;
			Serial.println("Kill.");
		}

		Serial.print("data[0]: ");
		Serial.println(data[0], HEX);

		forwardToRadio(CAN_POKB_BUS_stats, 1, data);

		sendPOKBTime = millis();
	}

	batteryCheck();
}

void batteryCheck() {
	double batteryVoltage = analogRead(BATTERY_VOLTAGE) / 276;
	Serial.print("batteryVoltage: ");
	Serial.println(batteryVoltage);

	if (batteryVoltage < 2.8) {
		digitalWrite(LOW_BATT, HIGH);
	}
	else {
		digitalWrite(LOW_BATT, LOW);
	}
}

// Radio Functions

void forwardToRadio(int canID, int dataLength, uint8_t data[]) {

	// Data Format: START_BYTE, START_BYTE, canID, dataLength, data (up to 8 bytes), checksum

	uint8_t temp[13] = { 0 };

	temp[0] = START_BYTE;
	temp[1] = START_BYTE;
	temp[2] = canID;
	temp[3] = dataLength;

	for (int i = 4, j = 0; j < dataLength; i++, j++) {
		temp[i] = data[j];
	}

	// Calculate checksum of [canID, dataLength, data] in case either canID or dataLength is erroraneous

	uint8_t crc = checksum(temp + 2, dataLength + 2);

	temp[dataLength + 4] = crc;

	for (int i = 0; i < dataLength + 5; i++) {
		Serial2.write(temp[i]);
	}

	Serial2.flush();
}

// Compare calculated crc and actual last byte 
// buffer[] is [canID, dataLength, data, checksum]

bool isValidCrc(uint8_t buffer[], int bufferLength) {
	uint8_t crc = checksum(buffer, bufferLength);

	if (crc == buffer[bufferLength]) {
		return true;
	}

	return false;
}

// Compute checksum based on buffer[] and bufferLength
// buffer[] is [canID, dataLength, data, checksum]

uint8_t checksum(uint8_t buffer[], uint8_t bufferLength) {
	uint8_t crc = 0;
	uint8_t i = 0;

	while (bufferLength != 0) {
		// Byte XOR
		crc = crc ^ buffer[i];
		i++;
		bufferLength--;
	}

	return crc;
}