/*
 Name:		POKB_Main.ino
 Created:	12/16/2017 9:55:06 PM
 Author:	RJ
*/

#include "XBee.h"
#include "defines.h"
#include "can_asv_defines.h"
#include <can.h>
#include <can_defines.h>

MCP_CAN CAN(CAN_Chip_Select);               //Set Chip Select to pin 8
unsigned char len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

static uint32_t xbee_loop = 0;
static uint32_t heartbeat_loop = 0;

uint8_t incomingByte = 0;
int no_data = 0;
bool software_kill = 0;

void setup() {
	// put your setup code here, to run once:
	Serial.begin(SERIAL_BAUDRATE);
	Serial1.begin(XBEE_BAUDRATE);
	xbee.setSerial(Serial1);
	pinMode(NMOS_CONTACTOR, OUTPUT);
	digitalWrite(NMOS_CONTACTOR, LOW);
	
	Serial.println("reset");

	CAN_init();
	Serial.println("CAN OK");

	xbee_loop = millis();
	heartbeat_loop = millis();

  pinMode(5, INPUT);
}

void loop() {
	if ((millis() - xbee_loop) > XBEE_TIMEOUT){
		xbee_receive();
    //Serial.print("Button: ");
    //Serial.println(digitalRead(5));
		xbee_loop = millis();
	}


	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT) {
		publishCAN_heartbeat();
		heartbeat_loop = millis();
	}
}

void On_Contactor() {
	digitalWrite(NMOS_CONTACTOR, HIGH);
	Serial.println("normal");
	software_kill = 0;
	no_data = 0;
}

void Kill_Contactor() {
	digitalWrite(NMOS_CONTACTOR, LOW);
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
				publishCAN_estop(false);
			}

			else {
				Kill_Contactor();
				publishCAN_estop(true);
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


//========== CAN ==========//

void CAN_init() {
START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 500k
#if DEBUG_MODE == NORMAL
		Serial.println("CAN init: OK");
#endif           
	}
	else {
#if DEBUG_MODE == NORMAL
		Serial.println("CAN init: FAILED");
		Serial.println("reinitialising CAN...");
		delay(1000);
#endif           
		goto START_INIT;
	}
}

void publishCAN_heartbeat()
{
	CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_POKB);
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

void publishCAN_estop(bool estop_status)
{
	CAN.setupCANFrame(buf, 0, 1, estop_status);
	CAN.sendMsgBuf(CAN_e_stop, 0, 1, buf);
}
