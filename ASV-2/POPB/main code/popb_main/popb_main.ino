#include "Arduino.h"
#include "can.h"
#include "can_asv_defines.h"

#define DEBUG_MODE true

/* Load Switch Pin Mappings */
#define LS_NAVTIC 46 //Q2_GATE
#define LS_POE_INJECTOR 45 //Q3_GATE
#define LS_RADAR 44 //Q4_GATE
#define LS_VELODYNE 42 //Q5_GATE
#define LS_E1 47 //Q6_GATE -- E1_GATE
#define LS_SICK 43 //Q7_GATE -- E2_GATE

/* Load Switch Bit Comparison */
#define ENABLE_RADAR 0x01
#define ENABLE_VELODYNE 0x02
#define ENABLE_SICK 0x04
#define ENABLE_NAVTIC 0x08
#define ENABLE_POE_INJECTOR 0x10
#define ENABLE_E1 0x20

#define STAGGER_TIME 250
#define CAN_HEARTBEAT_INTERVAL 500
#define CAN_STATUS_INTERVAL 1000
#define SERIAL_INTERVAL 2000

MCP_CAN CAN(8);
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };
uint32_t CanHeartbeatLoop = 0;
uint32_t CanStatusLoop = 0;
uint32_t SerialLoop = 0;

uint8_t LsControl = 0x1F; //Indicates which LS are turned on/off

void setup()
{
	Serial.begin(115200);
	Serial.println("This is POPB!");
	CAN_init();

	/* Initialise Load Switch Pins*/
	pinMode(LS_NAVTIC, OUTPUT);
	pinMode(LS_POE_INJECTOR, OUTPUT);
	pinMode(LS_RADAR, OUTPUT);
	pinMode(LS_VELODYNE, OUTPUT);
	pinMode(LS_E1, OUTPUT);
	pinMode(LS_SICK, OUTPUT);

	/* Startup Sequence*/
	//First off everything
	digitalWrite(LS_E1, LOW); //Spare E1 default off
	digitalWrite(LS_NAVTIC, LOW);
	digitalWrite(LS_POE_INJECTOR, LOW);
	digitalWrite(LS_RADAR, LOW);
	digitalWrite(LS_VELODYNE, LOW);
	digitalWrite(LS_SICK, LOW);

	//Staggered turn on
	digitalWrite(LS_NAVTIC, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(LS_RADAR, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(LS_POE_INJECTOR, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(LS_VELODYNE, HIGH);
	delay(STAGGER_TIME);
	digitalWrite(LS_SICK, HIGH);

}

void loop()
{
	// POPB Heartbeat
	if ((millis() - CanHeartbeatLoop) > CAN_HEARTBEAT_INTERVAL) {
		publishCanHB();
		CanHeartbeatLoop = millis();
	}

	if ((millis() - CanStatusLoop) > CAN_STATUS_INTERVAL) {
		publishCanStatus();
		CanStatusLoop = millis();
	}

	// Check for incoming CAN message
	if (checkCanMsg() == true) {
		// Update load switch states
		Serial.println("Updating load switch states");
		updateLoadSwitch();
	}

	if (DEBUG_MODE && ((millis() - SerialLoop) > SERIAL_INTERVAL)) {
		Serial.print("Status: ");
		Serial.println(LsControl, HEX);
		SerialLoop = millis();
	}
}

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
	uint8_t STATUS[1] = { LsControl };
	CAN.sendMsgBuf(CAN_POPB_stats, 0, 1, STATUS);
}

void publishCanHB() {
	uint8_t HB[1] = { HEARTBEAT_POPB };
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, HB);
}

boolean checkCanMsg() {
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		boolean mine = false;
		switch (id) {
		case CAN_POPB_control:
			LsControl = CAN.parseCANFrame(buf, 0, 1);
			mine = true;
			break;
		default:
			//Serial.println("Others");
			break;
		}
		CAN.clearMsg();
		return mine;
	}
	else
		return false;
}

void updateLoadSwitch() {
	if ((LsControl & ENABLE_E1) > 0) {
		digitalWrite(LS_E1, HIGH);
	}
	else {
		digitalWrite(LS_E1, LOW);
	}

	if ((LsControl & ENABLE_NAVTIC) > 0) {
		digitalWrite(LS_NAVTIC, HIGH);
	}
	else {
		digitalWrite(LS_NAVTIC, LOW);
	}

	if ((LsControl & ENABLE_POE_INJECTOR) > 0) {
		digitalWrite(LS_POE_INJECTOR, HIGH);
	}
	else {
		digitalWrite(LS_POE_INJECTOR, LOW);
	}

	if ((LsControl & ENABLE_RADAR) > 0) {
		digitalWrite(LS_RADAR, HIGH);
	}
	else {
		digitalWrite(LS_RADAR, LOW);
	}

	if ((LsControl & ENABLE_SICK) > 0) {
		digitalWrite(LS_SICK, HIGH);
	}
	else {
		digitalWrite(LS_SICK, LOW);
	}

	if ((LsControl & ENABLE_VELODYNE) > 0) {
		digitalWrite(LS_VELODYNE, HIGH);
	}
	else {
		digitalWrite(LS_VELODYNE, LOW);
	}
}