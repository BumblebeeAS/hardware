#include <Arduino.h>
#include <SPI.h>
#include <can.h>
#include "can_auv_define.h"

#define DEBUG_MODE true
#define DEBUG_MODE_INTENSE false

// Interval Periods

#define UPDATE_PCB_CONTROL_INTERVAL 5000

// CAN Setup

uint8_t chipSelect = 8;
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

MCP_CAN CAN(chipSelect);

// Interval Variables

uint32_t updatePcbControlTime = 0;

// Send PCB Control

uint8_t pcbControl = 0x01;	// Bit 0 to 4: SON, NAV, TAB, STB

// Receive Heartbeat, PCB Status

uint8_t heartbeat = 0x00;
uint8_t pcbStatus = 0x00; 

void setup()
{
	/* add setup code here */

	Serial.begin(115200);
	Serial.println("Power Control Board Test.");
	canInitialise();

	// CAN Acceptance Masks

	CAN.init_Mask(0, 0, 0x3FF);
	CAN.init_Mask(1, 0, 0x3FF);

	// CAN Acceptance Filter

	CAN.init_Filt(0, 0, CAN_heartbeat);
	CAN.init_Filt(1, 0, CAN_PCB_stats);
}

void loop()
{
	/* add main program code here */

	// Send PCB Control via CAN

	if ((millis() - updatePcbControlTime) > UPDATE_PCB_CONTROL_INTERVAL) {
		if (pcbControl >= 8) {
			pcbControl = 1;
		}
		else {
			pcbControl = pcbControl << 1;
		}
		sendPcbControl();
		updatePcbControlTime = millis();
	}

	// Receive CAN message

	receiveCanMessage();
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

void sendPcbControl() {
	uint8_t PCBControl[1] = { pcbControl };
	CAN.sendMsgBuf(CAN_PCB_Control, 0, 1, PCBControl);
}

boolean receiveCanMessage() {
	if (CAN.checkReceive() == CAN_MSGAVAIL) {
		CAN.readMsgBufID(&id, &len, buf);
		boolean messageForPcbTest = false;
		switch (id) {
		case CAN_heartbeat:
			heartbeat = CAN.parseCANFrame(buf, 0, 1);
			messageForPcbTest = true;
			if (DEBUG_MODE) {
				Serial.print("Heartbeat Received: ");
				Serial.println(heartbeat, HEX);
			}
			break;
		case CAN_PCB_stats:
			pcbStatus = CAN.parseCANFrame(buf, 0, 1);
			messageForPcbTest = true;
			if (DEBUG_MODE) {
				Serial.print("PCB Status Received: ");
				Serial.println(pcbStatus, HEX);
			}
			break;
		default:
			break;
		}

		CAN.clearMsg();
		return messageForPcbTest;
	}

	else {
		return false;
	}
}
