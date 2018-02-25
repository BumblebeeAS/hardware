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
uint32_t id = 0;

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

static uint32_t xbee_loop = 0;
static uint32_t heartbeat_loop = 0;
static uint32_t estop_loop = 0;
static uint32_t can_loop = 0;
static uint32_t posb_hb_loop = 0;

uint8_t incomingByte = 0;
int no_data = 0;
bool hard_kill = 0;		// Onboard
bool remote_kill = 0;	// Remote kill box
bool soft_kill = 0;		// From OCS

void setup() {
	
	// put your setup code here, to run once:
	Serial.begin(SERIAL_BAUDRATE);
	Serial1.begin(XBEE_BAUDRATE);
	xbee.setSerial(Serial1);
	pinMode(NMOS_CONTACTOR, OUTPUT);
	digitalWrite(NMOS_CONTACTOR, LOW);

	Serial.println("Hi I am POKB!");
	Serial.println("reset");

	CAN_init();
	Serial.println("CAN OK");

	xbee_loop = millis();
	heartbeat_loop = millis();
	estop_loop = millis();
	can_loop = millis();
	posb_hb_loop = millis();

  pinMode(HARDKILL_IN, INPUT);
}

void loop() {
	// Read incoming remote kill
	if ((millis() - xbee_loop) > XBEE_TIMEOUT){
		xbee_receive();
		xbee_loop = millis();
	}
	
	// Send current kill status via CAN
	if ((millis() - can_loop) > CAN_TIMEOUT)
	{
#ifdef _HACKJOB_
		readHardKill();
		if (hard_kill || remote_kill || soft_kill)
#else
		if (remote_kill || soft_kill)
#endif
		{
			publishCAN_estop(true);
		}
		else
		{
			publishCAN_estop(false);
		}
		can_loop = millis();
	}
	// Set current kill
	setKill();
	
	// Send POKB heartbeat via CAN
	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT) {
		publishCAN_heartbeat();
		heartbeat_loop = millis();
	}

	checkCANmsg();
}

void readHardKill() {
	hard_kill = digitalRead(HARDKILL_IN) ? false : true;
	/*
	Serial.print("KILL STAT: ");
	Serial.print(hard_kill);
	Serial.print(" ");
	Serial.println(remote_kill);*/
}

void setKill() {
	// Check POSB heartbeat for failsafe
	if ((millis() - posb_hb_loop) > FAILSAFE_TIMEOUT) {
		On_Contactor();
	}
	// On kill, if any source of kill is activated
	else if ((millis() - estop_loop) > ESTOP_TIMEOUT)
	{
		if (remote_kill || soft_kill)
		{
			Kill_Contactor();
		}
		else
		{
			On_Contactor();
		}
		estop_loop = millis();
	}
}

void On_Contactor() {
	digitalWrite(NMOS_CONTACTOR, HIGH);
	Serial.println("normal");
}

void Kill_Contactor() {
	digitalWrite(NMOS_CONTACTOR, LOW);
	Serial.println("KILL!");
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
				remote_kill = false;
			}

			else {
				remote_kill = true;
			}
			no_data = 0;
		}
	}
	else {
		no_data++;
		Serial.print("Count: ");
		Serial.println(no_data);

		if (no_data == 20) {
			remote_kill = true;
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

void checkCANmsg() {
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		switch (id) {
		case CAN_heartbeat:
			if (buf[0] == HEARTBEAT_POSB)
				posb_hb_loop = millis();
		case CAN_soft_e_stop:
			soft_kill = !buf[0];
			break;
		default:
			//Serial.println("Others");
			break;
		}
		CAN.clearMsg();
	}
}