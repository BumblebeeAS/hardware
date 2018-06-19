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
void retract_grabber();
void extend_grabber();
void manipulate();
void reset_manipulate();

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t maniControl = 0x00;	// Bit 0 to 7: RETXTXDX

uint32_t dropperTimer = 0;
uint32_t torpedoTopTimer = 0;
uint32_t torpedoBotTimer = 0;
uint32_t serialStatusTimer = 0;
uint32_t heartbeatTimer = 0;

int fired_dropper = 0;
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
	reset_manipulate();

	if ((millis() - serialStatusTimer) > SERIAL_STATUS_INTERVAL) {
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
			maniControl |= FIRE_TOP_TORPEDO;
		}
		else if (incomingByte == 50) //if 2 is typed in terminal, do bot torpedo
		{
			maniControl |= FIRE_BOT_TORPEDO;
		}
		else if (incomingByte == 51) //if 3 is typed in terminal, do dropper
		{
			maniControl |= FIRE_DROPPER;
		}
		else if (incomingByte == 52) //if 4 is typed in terminal, retract grabber
		{
			maniControl |= RETRACT_GRABBER;
		}
		else if (incomingByte == 53) //if 5 is typed in terminal, extend grabber
		{
			maniControl |= EXTEND_GRABBER;
		}

		manipulate();
		reset_manipulate();	
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
	//Mapped to pin 32
	//Turn on solenoid to fire dropper
	digitalWrite(MANI_3, HIGH);
	
	fired_dropper = 1;
	dropperTimer = millis();
}

void top_torpedo()
{
	//Mapped to pin 33
	//Turn on top solenoid to fire top torpedo
	digitalWrite(MANI_4, HIGH);
	
	fired_top = 1;
	torpedoTopTimer = millis();
}

void bot_torpedo()
{
	//Mapped to pin 34
	//Turn on bot solenoid to fire bot torpedo
	digitalWrite(MANI_5, HIGH);
	
	fired_bot = 1;
	torpedoBotTimer = millis();
}

void retract_grabber() 
{
	//Mapped to pin 41
	//Turn off solenoid to retract grabber (default state off)
	digitalWrite(MANI_6, LOW);
}

void extend_grabber()
{
	//Mapped to pin 41
	//Turn on solenoid to extend grabber
	digitalWrite(MANI_6, HIGH);
}

void manipulate() 
{
	if (maniControl & FIRE_TOP_TORPEDO) {
		top_torpedo();
		Serial.println("Fired top torpedo");
	}

	if (maniControl & FIRE_BOT_TORPEDO) {
		bot_torpedo();
		Serial.println("Fired bot torpedo");
	}

	if (maniControl & FIRE_DROPPER) {
		dropper();
		Serial.println("Fired dropper");
	}

	if (maniControl & RETRACT_GRABBER) {
		retract_grabber();
		Serial.println("Retracted grabber");
	}

	if (maniControl & EXTEND_GRABBER) {
		extend_grabber();
		Serial.println("Extended grabber");
	}

	maniControl = 0;
}

void reset_manipulate()
{
	if (fired_dropper && (millis() - dropperTimer) > DROPPER_INTERVAL)
	{
		//Turn off solenoid to reload dropper
		digitalWrite(MANI_3, LOW);
		fired_dropper = 0;
		Serial.println("Closed dropper");
	}

	if (fired_top && (millis() - torpedoTopTimer) > TORPEDO_INTERVAL)
	{
		//Turn off top solenoid
		digitalWrite(MANI_4, LOW);
		fired_top = 0;
		Serial.println("Closed top torpedo");
	}

	if (fired_bot && (millis() - torpedoBotTimer) > TORPEDO_INTERVAL)
	{
		//Turn off bot solenoid
		digitalWrite(MANI_5, LOW);
		fired_bot = 0;
		Serial.println("Closed bot torpedo");
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


