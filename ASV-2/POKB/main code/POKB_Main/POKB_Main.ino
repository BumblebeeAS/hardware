/*
 Name:		POKB_Main.ino
 Created:	12/16/2017 9:55:06 PM
 Author:	RJ
*/

#include "XBee.h"
#include "defines.h"

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

static uint32_t xbee_loop = 0;
uint8_t incomingByte = 0;
int no_data = 0;
bool software_kill = 0;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(SERIAL_BAUDRATE);
	Serial1.begin(XBEE_BAUDRATE);
	xbee.setSerial(Serial1);
	pinMode(11, OUTPUT);
	digitalWrite(11, LOW);

	pinMode(5, OUTPUT);
	digitalWrite(5, LOW);

	Serial.println("reset");

	xbee_loop = millis();
}

void loop() {
	if ((millis() - xbee_loop) > XBEE_DELAY){
		xbee_receive();
		xbee_loop = millis();
	}
}

void On_Contactor() {
	digitalWrite(11, HIGH);
	digitalWrite(5, HIGH);
	Serial.println("normal");
	software_kill = 0;
	no_data = 0;
}

void Kill_Contactor() {
	digitalWrite(11, LOW);
	digitalWrite(5, LOW);
	Serial.println("KILL!");
	software_kill = 1;
	no_data = 0;
}

void xbee_receive(){
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		Serial.println("Available");
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE){
			xbee.getResponse().getZBRxResponse(rx);
			// get data
			incomingByte = rx.getData(0);
			Serial.print("Incoming Byte: ");
			Serial.println(incomingByte, HEX);

			if (incomingByte == 0x15) {
				On_Contactor();
			}

			else {
				Kill_Contactor();
			}
		}
	}

	else {
		no_data++;
		Serial.print("Count: ");
		Serial.println(no_data);

		if (no_data == 20) {
			Kill_Contactor();
			Serial.println("Connection Time-out KILL");
		}
		Serial.println("Nothing Available");
	}
}