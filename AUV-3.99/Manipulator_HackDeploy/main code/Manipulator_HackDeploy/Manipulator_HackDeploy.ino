//###################################################
//###################################################
//
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####    
//
//
//Manipulator_HackDeploy for BBAUV 3.99
//Firmware Version :             v1.0
////
// Written by Ren Jie
// Change log v0.0:
//
//###################################################
//###################################################


#include <can_defines.h>
#include "can_auv_define.h"
#include "define.h"
#include <can.h>
#include <Arduino.h>


void CAN_init();
void publishCanHB();
boolean receiveCanMessage();
void manipulators_init();
void dropper();
void top_torpedo();
void bot_torpedo();
void manipulate();

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t maniControl = 0x00;	// Bit 0 to 7: XXTXTXDX

uint32_t dropperTimer = 0;
uint32_t torpedoTopTimer = 0;
uint32_t torpedoBotTimer = 0;
uint32_t serialStatusTimer = 0;
uint32_t heartbeatTimer = 0;

int reload = 0;
int fired_top = 0;
int fired_bot = 0;

int incomingByte = 0;   // for incoming serial data; test code

void setup() {
	Serial.begin(115200);
	Serial.println("Hi, I'm Hackjob Manipulator!");
	CAN_init(); //initialise CAN
	heartbeatTimer = serialStatusTimer = millis();
	manipulators_init();
}

void loop()
{
	// Send MANI Heartbeat via CAN
	if ((millis() - heartbeatTimer) > HEARTBEAT_TIMEOUT) {
		publishCanHB();
		heartbeatTimer = millis();
	}

	//If received msg from CAN
	if (receiveCanMessage()) {
		manipulate();
	}

	if (DEBUG_MODE && ((millis() - serialStatusTimer) > SERIAL_STATUS_INTERVAL)) {
		Serial.print("Manipulator Status: ");
		for (int i = 7; i >= 0; i--)
		{
			bool b = bitRead(maniControl, i);
			Serial.print(b);
		}
		Serial.println();
		serialStatusTimer = millis();
	}

	//Test code using Serial 
	if (Serial.available() > 0) {
		// read the incoming byte:
		incomingByte = Serial.read();

		// say what you got:
		Serial.print("I received: ");
		Serial.println(incomingByte, DEC);

		if (incomingByte == 49) //if 1 is typed in terminal, do top torpedo
		{
			top_torpedo();
		}
		else if (incomingByte == 50) //if 2 is typed in terminal, do bot torpedo
		{
			bot_torpedo();
		}
		else if (incomingByte == 51) //if 3 is typed in terminal, do dropper
		{
			dropper();
		}
	}
}

void manipulators_init()
{
	//Initialize Manipulators
	pinMode(MANI_1, OUTPUT);
	pinMode(MANI_2, OUTPUT);
	pinMode(MANI_3, OUTPUT);
	pinMode(MANI_4, OUTPUT);
	pinMode(MANI_5, OUTPUT);
	pinMode(MANI_6, OUTPUT);
	pinMode(MANI_7, OUTPUT);
	pinMode(MANI_8, OUTPUT);
	pinMode(MANI_9, OUTPUT);
	pinMode(TORP1, OUTPUT);
	pinMode(TORP2, OUTPUT);

	//Set all to low
	digitalWrite(MANI_1, LOW);
	digitalWrite(MANI_2, LOW);
	digitalWrite(MANI_3, LOW);
	digitalWrite(MANI_4, LOW);
	digitalWrite(MANI_5, LOW);
	digitalWrite(MANI_6, LOW);
	digitalWrite(MANI_7, LOW);
	digitalWrite(MANI_8, LOW);
	digitalWrite(MANI_9, LOW);
	digitalWrite(TORP1, LOW);
	digitalWrite(TORP1, LOW);
}

void dropper()
{
	//Mapped to pin 34
	//Turn on solenoid to fire dropper
	digitalWrite(MANI_5, HIGH);
	
	dropperTimer = millis();
	reload = 0;

	while (reload == 0)
	{
		if (millis() - dropperTimer > DROPPER_INTERVAL)
		{
			//Turn off solenoid to reload dropper
			digitalWrite(MANI_5, LOW);
			reload = 1;
		}
	}
}

void top_torpedo()
{
	//GPIO pin 22
	//Turn on top solenoid to fire top torpedo
	digitalWrite(TORP1, HIGH);
	fired_top = 1;

	torpedoTopTimer = millis();

	while (fired_top == 1)
	{
		if (millis() - torpedoTopTimer > TORPEDO_INTERVAL)
		{
			//Turn off top solenoid
			digitalWrite(TORP1, LOW);
			fired_top = 0;
		}
	}
}

void bot_torpedo()
{
	//GPIO pin 23
	//Turn on bot solenoid to fire bot torpedo
	digitalWrite(TORP2, HIGH);
	fired_bot = 1;

	torpedoBotTimer = millis();

	while (fired_bot == 1)
	{
		if (millis() - torpedoBotTimer > TORPEDO_INTERVAL)
		{
			//Turn off bot solenoid
			digitalWrite(TORP2, LOW);
			fired_bot = 0;
		}
	}
}

void manipulate() 
{
	if (maniControl & FIRE_TOP_TORPEDO) {
		top_torpedo();
	}

	else if (maniControl & FIRE_BOT_TORPEDO) {
		bot_torpedo();
	}

	else if (maniControl & FIRE_DROPPER) {
		dropper();
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

void publishCanHB() {
	uint8_t HB[1] = { HEARTBEAT_MANI }; //HEARTBEAT_MANI
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, HB);
}

boolean receiveCanMessage() {
	if (CAN.checkReceive() == CAN_MSGAVAIL) {
		CAN.readMsgBufID(&id, &len, buf);
		boolean messageForMani = false;
		switch (id) {
		case CAN_manipulator:
			maniControl = CAN.parseCANFrame(buf, 0, 1);
			messageForMani = true;
			if (DEBUG_MODE) {
				Serial.print("Mani Control Received: ");
				Serial.println(maniControl, HEX);
			}
			break;
		default:
			break;
		}
		CAN.clearMsg();
		return messageForMani;
	}

	else {
		return false;
	}
}


