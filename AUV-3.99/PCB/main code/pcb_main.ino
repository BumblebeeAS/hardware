#include <Arduino.h>
#include <SPI.h>
#include <can.h>
#include "can_auv_define.h"

#define DEBUG_MODE true
#define DEBUG_MODE_INTENSE false

// Note: For program upload, Power Control Board is 2nd largest port on quad-FTDI

// To be Switched On/ Off

// SON: Sonar
// NAV: Navigation
// TAB: Thruster Actuator
// STB: Sensor Telemetry

// Load Switch (NMOS)/ PMOS to ATmega2560 Digital Pin Mapping 

#define SON_GATE 25	// Load Switch (NMOS), Active High
#define NAV_GATE 28	// Load Switch (NMOS), Active High
#define TAB_GATE 54	// PMOS, Active Low
#define STB_GATE 57	// PMOS, Active Low

// Load Switch (NMOS)/ PMOS Bit Comparison

#define ENABLE_TAB 0x01
#define ENABLE_STB 0x02
#define ENABLE_NAV 0x04
#define ENABLE_SON 0x08

// Interval Periods

#define STAGGER_TIME 500
#define CAN_HEARTBEAT_INTERVAL 500
#define CAN_STATUS_INTERVAL 1000
#define SERIAL_STATUS_INTERVAL 5000

// CAN Setup

uint8_t chipSelect = 8;
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = {0};

MCP_CAN CAN(chipSelect);

// CAN Message IDs

// (CAN Message ID): (Data), (Sender), (Receiver)
// CAN_PCB_Control: pcbControl, Single Board Computer, Power Control Board
// CAN_PCB_stats: pcbStatus, Power Control Board, Sensor Telemetry Board
// CAN_heartbeat: pcbHeartbeat, Power Control Board, Sensor Telemetry Board

// Interval Variables

uint32_t canHeartbeatTime = 0;
uint32_t canStatusTime = 0;
uint32_t serialStatusTime = 0; 

// Switch On/Off Indication

uint8_t pcbControl = 0x0F;	// Bit 0 to 3: SON, NAV, TAB, STB

void setup()
{
	/* add setup code here */

	Serial.begin(115200);
	Serial.println("Power Control Board.");
	canInitialise();

	// CAN Acceptance Masks

	CAN.init_Mask(0, 0, 0x3FF);	
	CAN.init_Mask(1, 0, 0x3FF);

	// CAN Acceptance Filter

	CAN.init_Filt(0, 0, CAN_PCB_Control);

	// PinMode Configuration
	
	pinMode(SON_GATE, OUTPUT);
	pinMode(NAV_GATE, OUTPUT);
	pinMode(TAB_GATE, OUTPUT);
	pinMode(STB_GATE, OUTPUT);

	// Startup Sequence

	// Switch off SON, NAV, TAB and STB

	digitalWrite(SON_GATE, LOW);
	digitalWrite(NAV_GATE, LOW);
	digitalWrite(TAB_GATE, HIGH);
	digitalWrite(STB_GATE, HIGH);

	// Switch on SON, NAV, TAB and STB in sequence
	delay(STAGGER_TIME);
	digitalWrite(SON_GATE, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(NAV_GATE, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(STB_GATE, LOW);
	delay(5000);
	digitalWrite(TAB_GATE, LOW);
	
}

void loop()
{
	/* add main program code here */

	// Send PCB Heartbeat via CAN

	if ((millis() - canHeartbeatTime) > CAN_HEARTBEAT_INTERVAL) {
		sendCanHeartbeat();
		canHeartbeatTime = millis();
	}

	// Send PCB Status via CAN

	if ((millis() - canStatusTime) > CAN_STATUS_INTERVAL) {
		sendCanStatus();
		canStatusTime = millis();
	}

	// Receive CAN message

	if (receiveCanMessage()) {
		updateSwitches();
	}

	// Print PCB Status via Serial

	if (DEBUG_MODE && ((millis() - serialStatusTime) > SERIAL_STATUS_INTERVAL)) {
		Serial.print("PCB Status: ");
		Serial.println(pcbControl, HEX);
		serialStatusTime = millis();
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

void sendCanHeartbeat() {
	uint8_t pcbHeartbeat[1] = { HEARTBEAT_PCB };
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, pcbHeartbeat);
}

void sendCanStatus() {
	uint8_t pcbStatus[1] = { pcbControl };
	CAN.sendMsgBuf(CAN_PCB_stats, 0, 1, pcbStatus);
}

boolean receiveCanMessage() {
	if (CAN.checkReceive() == CAN_MSGAVAIL) {
		CAN.readMsgBufID(&id, &len, buf);
		boolean messageForPcb = false;
		switch (id) {
		case CAN_PCB_Control:
			pcbControl = CAN.parseCANFrame(buf, 0, 1);
			messageForPcb = true;
			if (DEBUG_MODE) {
				Serial.print("PCB Control Received: ");
				Serial.println(pcbControl, HEX);
			}
			break;
		default:
			break;
		}

		CAN.clearMsg();
		return messageForPcb;
	}

	else {
		return false;
	}
}

void updateSwitches() {
	if (pcbControl & ENABLE_SON){
		digitalWrite(SON_GATE, HIGH);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("SON: H | ");
		}
	}

	else {
		digitalWrite(SON_GATE, LOW);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("SON: L | ");
		}
	}

	if (pcbControl & ENABLE_NAV) {
		digitalWrite(NAV_GATE, HIGH);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("NAV: H | ");
		}
	}

	else {
		digitalWrite(NAV_GATE, LOW);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("NAV: L | ");
		}
	}

	if (pcbControl & ENABLE_TAB) {
		digitalWrite(TAB_GATE, LOW);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("TAB: H | ");
		}
	}

	else {
		digitalWrite(TAB_GATE, HIGH);
		if (DEBUG_MODE_INTENSE) {
			Serial.print("TAB: L | ");
		}
	}

	if (pcbControl & ENABLE_STB) {
		digitalWrite(STB_GATE, LOW);
		if (DEBUG_MODE_INTENSE) {
			Serial.println("STB: H ");
		}
	}

	else {
		digitalWrite(STB_GATE, HIGH);
		if (DEBUG_MODE_INTENSE) {
			Serial.println("STB: L ");
		}
	}

	if (DEBUG_MODE) {
		Serial.print("PCB Status Updated: ");
		Serial.println(pcbControl, HEX);
	}
}
