#include <XBee.h>
#include <can.h>
#include "can_asv_defines.h"

// COM Port: Third smallest/ second largest

#define CAN_CHIP_SELECT 8

#define SERIAL_BAUD_RATE 115200
#define XBEE_BAUD_RATE 115200

#define CONTACTOR_CONTROL 11  // NMOS, Active High 
#define ONBOARD_SWITCH 21      
#define KILL 31

#define RECEIVE_REMOTE_KILL_TIMEOUT 500
#define RECEIVE_POSB_HEARTBEAT_TIMEOUT 3000
#define UPDATE_CONTACTOR_TIMEOUT 100
#define SEND_POKB_HEARTBEAT_TIMEOUT 500
#define SEND_POKB_STATUS_TIMEOUT 1000

// CAN Setup
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];

MCP_CAN CAN(CAN_CHIP_SELECT);

// XBee Setup
XBee xbee = XBee();
ZBRxResponse zbRx = ZBRxResponse();
uint8_t inByte = 0;
int noData = 0;

// Time Counter Variables
uint32_t receiveRemoteKillTime = 0;
uint32_t receivePOSBHeartbeatTime = 0;
uint32_t updateContactorTime = 0;
uint32_t sendPOKBHeartbeatTime = 0;
uint32_t sendPOKBStatusTime = 0;

// Control Variables
bool onboardKill = false;
bool remoteKill = false;
bool softwareKill = false;

void setup() {
	// Put your setup code here, to run once:
	Serial.begin(SERIAL_BAUD_RATE);
	Serial1.begin(XBEE_BAUD_RATE);

	xbee.setSerial(Serial1);

	pinMode(CONTACTOR_CONTROL, OUTPUT);
	digitalWrite(CONTACTOR_CONTROL, LOW);

	pinMode(ONBOARD_SWITCH, INPUT);

	Serial.println("Plenty of Kill Board.");

	canInitialise();

	receiveRemoteKillTime = millis();
	receivePOSBHeartbeatTime = millis();
	updateContactorTime = millis();
	sendPOKBHeartbeatTime = millis();
	sendPOKBStatusTime = millis();
}

void loop() {
	// Receive Onboard Kill via ATmega 2560 Input Pin
	onboardKill = !digitalRead(ONBOARD_SWITCH);

	// Receive Remote Kill via Xbee
	if ((millis() - receiveRemoteKillTime) > RECEIVE_REMOTE_KILL_TIMEOUT) {
		receiveRemoteKill();
		receiveRemoteKillTime = millis();
	}

	// Receive Software Kill and POSB Heartbeat via CAN
	receiveCanMessage();

	// Update contactor
	updateContactor();

	// Send POKB Heartbeat via CAN
	if ((millis() - sendPOKBHeartbeatTime) > SEND_POKB_HEARTBEAT_TIMEOUT) {
		sendPOKBHeartbeat();
		sendPOKBHeartbeatTime = millis();
	}

	// Send POKB Status via CAN
	if ((millis() - sendPOKBStatusTime) > SEND_POKB_STATUS_TIMEOUT) {
		if (onboardKill || remoteKill || softwareKill) {
			sendPOKBStatus(true);
		}

		else {
			sendPOKBStatus(false);
		}

		sendPOKBStatusTime = millis();
	}
}

void canInitialise() {
START_INIT:
	if (CAN.begin(CAN_1000KBPS) == CAN_OK) {
		Serial.println("CAN Bus: Initialisation successful.");
	}

	else {
		Serial.println("CAN Bus: Initialisation failed.");
		Serial.println("CAN Bus: Re-initialising.");
		delay(1000);
		goto START_INIT;
	}

	Serial.println("Starting transmission...");
}

void receiveRemoteKill() {
	// Continuously read packets
	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		// Serial.println("Response available.");

		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {

			// Got zb rx response
			// Fill zb rx response object
			xbee.getResponse().getZBRxResponse(zbRx);

			// Get data
			inByte = zbRx.getData(0);
			Serial.print("inByte: ");
			Serial.println(inByte, HEX);

			remoteKill = (inByte == 0x15) ? false : true;
			noData = 0;
		}
	}

	else {
		noData++;
		Serial.print("noData: ");
		Serial.println(noData);

		if (noData >= 20) {
			remoteKill = true;
			noData = 0;
			Serial.println("Connection timeout kill.");
		}
	}
}

void receiveCanMessage() {
	if (CAN.checkReceive() == CAN_MSGAVAIL) {
		CAN.readMsgBufID(&id, &len, buf);

		switch (id) {
		case CAN_heartbeat:
			if (buf[0] == HEARTBEAT_POSB)
				receivePOSBHeartbeatTime = millis();
			break;
		case CAN_soft_e_stop:
			softwareKill = !buf[0];
			break;
		default:
			break;
		}

		CAN.clearMsg();
	}
}

void updateContactor() {
	// Failsafe
	// Check POSB Heartbeat 

	/*
	if ((millis() - receivePOSBHeartbeatTime) > RECEIVE_POSB_HEARTBEAT_TIMEOUT) {
		// digitalWrite(CONTACTOR_CONTROL, LOW);
	}
	*/

	// else if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {
	if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {
		// Serial.print("Onboard Kill: ");
		// Serial.print(onboardKill);
		// Serial.print(" | ");
		// Serial.print("Remote Kill: ");
		// Serial.print(remoteKill);
		// Serial.print(" | ");
		// Serial.print("Software Kill: ");
		// Serial.println(softwareKill);

		if (onboardKill || remoteKill || softwareKill) {
			digitalWrite(CONTACTOR_CONTROL, LOW);
			digitalWrite(KILL, HIGH);
			// Serial.println("Contactor switched off.");
		}

		else {
			digitalWrite(CONTACTOR_CONTROL, HIGH);
			digitalWrite(KILL, LOW);
			// Serial.println("Contactor switched on.");
		}

		updateContactorTime = millis();
	}
}

void sendPOKBHeartbeat() {
	CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_POKB);
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

void sendPOKBStatus(bool pokbStatus) {
	CAN.setupCANFrame(buf, 0, 1, pokbStatus);
	CAN.sendMsgBuf(CAN_e_stop, 0, 1, buf);
}
