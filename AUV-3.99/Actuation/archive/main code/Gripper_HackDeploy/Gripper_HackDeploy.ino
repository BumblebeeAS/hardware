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
//Gripper_HackDeploy for BBAUV 3.99
//Firmware Version :             v1.0
////
// Written by Chong Yu
// Change log v1.0: 
//
//###################################################
//###################################################

#include <can_defines.h>
#include "can_auv_define.h"
#include "define.h"
#include <can.h>
#include <Arduino.h>
#include <Servo.h>

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t controlByte = 0x00;	// Bit 0: {0 - Close, 1 - Open}
uint32_t serialStatusTimer = 0;
uint32_t heartbeatTimer = 0;
Servo gripper;

void setup() {
	Serial_Init();		//Initialize Serial Port - 115200 baud
	Gripper_Init();		//Initialize Gripper
	CAN_init();			//Initialize CAN
	heartbeatTimer = serialStatusTimer = millis();
	Serial.println("=== READY ===");
}

void loop() {
	// Send Heartbeat via CAN
	if ((millis() - heartbeatTimer) > HEARTBEAT_TIMEOUT) {
		publishCanHB();
		heartbeatTimer = millis();
	}
	
	//If received msg from CAN
	if (receiveCanMessage()) {
		manipulate(controlByte);
	}
	
	//Debug with serial
	if(Serial.available()){
		byte data = Serial.read();
		if(data == '1'){
			manipulate(OPEN_GRIPPER);
		}
		else if(data == '2'){
			manipulate(CLOSE_GRIPPER);
		}
	}
}

void Serial_Init(){
	Serial.begin(115200);
	Serial.println("Hello, I'm AUV Gripper.");
}

void Gripper_Init(){
	gripper.attach(GRIP_PIN);
	gripper.writeMicroseconds(1000);
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
		boolean isGripperMessage = false;
		switch (id) {
			case CAN_manipulator:
			controlByte = CAN.parseCANFrame(buf, 0, 1);
			isGripperMessage = true;
			if (DEBUG_MODE) {
				Serial.print("Mani Control Received: ");
				Serial.println(controlByte, HEX);
			}
			break;
			default:
			break;
		}
		CAN.clearMsg();
		return isGripperMessage;
	}

	else {
		return false;
	}
}

void manipulate(byte status){
	if(status == OPEN_GRIPPER){
		gripper.writeMicroseconds(1900);
		Serial.println("Gripper Open");
	}
	else if(status == CLOSE_GRIPPER){
		gripper.writeMicroseconds(1100);
		Serial.println("Gripper Close");
	}
}