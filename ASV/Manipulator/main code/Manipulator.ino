#include <can.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <IMU.h>
#include <Sweeper.h>
#include "can_asv_defines.h"
#include <Actuator.h>


#define Chip_Select 8

MCP_CAN CAN(Chip_Select);
IMU imu;
Sweeper sweep(5);
Actuator actuator;

// CAN variables
uint8_t len = 0;
uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//Time variables
uint32_t heartbeat_loop = 0;

uint8_t enable=0;
uint8_t mani_state = 0;
uint8_t target_angle = 0;
uint8_t stepper_pos = 0;
uint8_t checksum = 0;


void setup () {

	Serial.begin(115200);
	CAN_init();
	imu.init();
  imu.correctToZero();          // run only when fitting to new position
  sweep.attach(9);              // pin connection
	actuator.init();

	heartbeat_loop = millis();
	
}

void loop() {
	
	sendHeartBeat();

	checkCANmsg();    // receive CAN message and compute checksum
					  // and move data out from buffer

	ENABLE();		  // actuate stuff
	        
} 

void CAN_init(){
START_INIT:
    if (CAN_OK == CAN.begin(CAN_1000KBPS)) {          // init can bus : baudrate = 500k
        // Serial.println("CAN init: OK");         
    }
    else {       
      // Serial.println("CAN init: FAILED");
      // Serial.println("reinitialising CAN...");
      // delay(1000); 
   	  goto START_INIT;
    }    

    CAN.init_Mask(0, 0, 0x3ff);                     // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x3ff);
    CAN.init_Filt(0, 0, CAN_manipulator);			// only the ones u filter you will receive(?
    												// if filtering for multi, use CAN.getId() for checking 
    												// which part is it for 
}
void checkCANmsg(){	
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		//read where is it from
		for (int x=0; x<5;x++) {		// clear buffer
     		buf[x] = 0;
    	}

		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		
		enable = buf[0];
		mani_state = buf[1];
		target_angle = buf[2];
		stepper_pos = buf[3];

		checksum = buf[0]^buf[1]^buf[2]^buf[3];
		buf[4] = checksum;

		CAN.setupCANFrame(buf,4,1,checksum);
		CAN.sendMsgBuf(CAN_manipulator,0,0,5,buf);
	}
}

void sendHeartBeat() {
	if ( (millis() - heartbeat_loop) > 500) {  
		CAN.setupCANFrame(buf,0,1,2);
		CAN.sendMsgBuf(CAN_heartbeat,0,1,buf);
		heartbeat_loop = millis();
		buf[0]=0;
	}
}

void ENABLE() {
	uint8_t val,diff = 0, shoot;

	for (val=1; val<16; val<<=1) {
		switch (enable & val) {
			case 1: 
				//acoustic
				if ( (bitRead(mani_state,0)) == 1 ) {
					actuator.retrieveAcoustic(LINEAR);
					actuator.retrieveAcoustic(ROTARY);
				} if( (bitRead(mani_state,1)) == 1) {
					actuator.deployAcoustic(ROTARY);					
				} if( (bitRead(mani_state,2)) == 1 ) {
					actuator.deployAcoustic(LINEAR);
				}
				break;
			case 2: 
				//shooter
				shoot = mani_state>>3;   // right shift mani_state such that shoot contains only bit for shooter
				break;
			case 4:
				//servo
				imu.readAccTempGyro();
				diff = (round)(975 / 90 * imu.correctError() + 1525);
				sweep.update(target_angle, diff);
				break;
			case 8:	
				//stepper

			default:
				target_angle = 19;
				break;
		}			
	}
}
