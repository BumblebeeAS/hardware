//#include <XBee.h>
#include <SoftwareSerial.h>
#include "can_asv_defines.h"
//#include "defines.h"

// create the XBee object
//XBee xbee = XBee();
//uint8_t payload[] = { 0, 0 };
//XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B9775);
//
//ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
//ZBTxStatusResponse txStatus = ZBTxStatusResponse();

//static uint32_t xbee_loop = 0;
//int8_t sentByte = 0x15;
//uint8_t inByte;

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
	// put your setup code here, to run once:
	Serial.begin(SERIAL_BAUD_RATE);
	radioSerial.begin(N2420_BAUD_RATE);
	//xbee.setSerial(mySerial);

	pinMode(REMOTE_KILL, INPUT);
	//xbee_loop = millis();

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

		//payload[0] = sentByte;
		//Serial.print("payload: ");
		//Serial.println(payload[0], HEX);

		//xbee.send(zbTx);

		//// after sending a tx request, we expect a status response
		//// wait up to half a second for the status response
		//if (xbee.readPacket(100))
		//{
		//	// got a response!
		//	// should be a znet tx status              
		//	if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
		//	{
		//		xbee.getResponse().getZBTxStatusResponse(txStatus);
		//		// get the delivery status, the fifth byte
		//		/*if (txStatus.getDeliveryStatus() == SUCCESS)
		//		{
		//		// success.  time to celebrate
		//		//flashLed(statusLed, 5, 10);
		//		Serial.println("Ack");
		//		}
		//		else
		//		{
		//		// the remote XBee did not receive our packet. is it powered on?
		//		//flashLed(errorLed, 1, 50);
		//		Serial.println("No Acknowledgement");
		//		}
		//		*/
		//	}
		//}
		//else
		//{
		//	// local XBee did not provide a timely TX Status Response -- should not happen
		//	//flashLed(errorLed, 5, 50);
		//	Serial.println("Sender Error");
		//}

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