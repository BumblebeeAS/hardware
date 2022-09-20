#include <can.h>
#include <can_asv3_defines.h>
#include "defines.h"

MCP_CAN CAN(8);
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];
uint32_t autoResetLoop = 0;
uint32_t CanHeartbeatLoop = 0;
uint32_t CanStatusLoop = 0;
uint32_t SerialLoop = 0;
uint8_t LsControl = 0x07; //Indicates which LS are turned on/off

void setup()
{
	Serial.begin(115200);
	Serial.println("This is POPB!");

  /* Initialise Load Switch Pins*/
  pinMode(LS_NAVTIC, OUTPUT);
  pinMode(LS_OUSTER, OUTPUT);
  pinMode(LS_VELODYNE, OUTPUT);

  /* Startup Sequence*/
  //First off everything
  digitalWrite(LS_NAVTIC, LOW);
  digitalWrite(LS_OUSTER, LOW);
  digitalWrite(LS_VELODYNE, LOW);

  //Staggered turn on
  digitalWrite(LS_NAVTIC, HIGH);
  delay(STAGGER_TIME);
  digitalWrite(LS_OUSTER, HIGH);
  delay(STAGGER_TIME);
  digitalWrite(LS_VELODYNE, HIGH);
  
	CAN_init();

	// CAN Masking
	CAN.init_Mask(0, 0, 0x3FF);
	CAN.init_Mask(1, 0, 0x3FF);
	CAN.init_Filt(0, 0, CAN_POPB_CONTROL);

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
	if (checkCanMsg()) {
		// Update load switch states
		if (DEBUG_MODE) 
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
	CAN.setupCANFrame(buf, 0, 1, LsControl);
	CAN.sendMsgBuf(CAN_POPB_STATS, 0, 1, buf);
}

void publishCanHB() {
  CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_POPB);
	CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

boolean checkCanMsg() {
	//Serial.println("checking CAN");
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		boolean mine = false;
		switch (id) {
		case CAN_POPB_CONTROL:
			LsControl = CAN.parseCANFrame(buf, 0, 1);
			mine = true;
			if (DEBUG_MODE)
				Serial.println(LsControl);
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
	if ((LsControl & ENABLE_NAVTIC) > 0) {
		digitalWrite(LS_NAVTIC, HIGH);
		if (DEBUG_MODE)
			Serial.print("NAV:H | ");
	}
	else {
		digitalWrite(LS_NAVTIC, LOW);
		if (DEBUG_MODE)
			Serial.print("NAV:L | ");
	}

	if ((LsControl & ENABLE_OUSTER) > 0) {
		digitalWrite(LS_OUSTER, HIGH);
		if (DEBUG_MODE)
			Serial.print("OUSTER:H | ");
	}
	else {
		digitalWrite(LS_OUSTER, LOW);
		if (DEBUG_MODE)
			Serial.print("OUSTER:L | ");
	}

	if ((LsControl & ENABLE_VELODYNE) > 0) {
		digitalWrite(LS_VELODYNE, HIGH);
		if (DEBUG_MODE)
			Serial.print("VELO:H | ");
	}
	else {
		digitalWrite(LS_VELODYNE, LOW);
		if (DEBUG_MODE)
			Serial.print("VELO:L | ");
	}
	if (DEBUG_MODE)
		Serial.println(LsControl, HEX);
}
