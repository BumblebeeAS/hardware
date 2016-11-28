#include <can.h>
#include <SPI.h>
#include <Wire.h>
#include <IMU.h>
#include <Sweeper.h>
#include "can_asv_defines.h"
#include <Actuator.h>
#include <step.h>

//pin definition
#define CAN_CHIP_SELECT 8

#define UPDATE_INTERVAL 5				// update interval for servo

//mask values for decoding CAN message
#define ACOUSTIC 1
#define SHOOTER  2
#define SWEEPER  4
#define STEPPER  8

// #define IMU_Ratio_Constant 10.83333333	// 1 step mapped to 1 deg

MCP_CAN CAN(CAN_CHIP_SELECT);
IMU imu;
Sweeper sweep;
Actuator actuator;
Step stepper; 

//Time variables
uint32_t heartbeat_loop = 0;

void setup () {
	Serial.begin(115200);
 
	CAN_init();
	imu.init();
	sweep.attach(10);
	actuator.init();
	stepper.init();
	heartbeat_loop = millis();
}

void loop() {
  uint8_t checkSum = 0, len = 0;
  uint8_t buf[8] = {0};

  //send manipulator hearbeat every 500ms
  if ((millis() - heartbeat_loop) >= 500){
	sendHeartBeat();
	heartbeat_loop = millis();
  }

  //check if CAN message available and perform respective funtions
  if (CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
    decodeCANMsg(buf);
    sendCheckSum(buf);
  }

  //move stepper and servo motor
  moveMotor();
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

    CAN.init_Mask(0, 0, 0x3ff);               // there are 2 mask in mcp2515, you need to set both of them
    CAN.init_Mask(1, 0, 0x3ff);
	CAN.init_Filt(0, 0, CAN_manipulator);			// only the ones u filter you will receive(?
    											                   	// if filtering for multi, use CAN.getId() for checking 
    											                   	// which part is it for 
}

void sendHeartBeat() {
	uint8_t buf[1];
	CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_Manipulator);
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}
  
void decodeCANMsg(const uint8_t* buf) {
	uint8_t mask, shoot;

	for (mask = ACOUSTIC; mask <= STEPPER; mask<<=1) {
		switch (buf[0] & mask) {
		case ACOUSTIC: 
			//acoustic
			if ((bitRead(buf[1],0)) == 1) {
				actuator.actuateAcoustic(ROTATE_UP);
			}
			else{
				actuator.stopActuation(ROTATE_UP);
			}
			if( (bitRead(buf[1],1)) == 1) {
				actuator.actuateAcoustic(ROTATE_DOWN);          
			}
			else{
				actuator.stopActuation(ROTATE_DOWN);
			}
			if( (bitRead(buf[1],2)) == 1) {
				actuator.actuateAcoustic(LINEAR);
			}
			else{
				actuator.stopActuation(LINEAR);
			}
			break;
		case SHOOTER: 
			//shooter
			shoot = buf[1] >> 3;   // right shift mani_state such that shoot contains only bit for shooter
			for(uint8_t mask = 1; mask < 0x10; mask<<=1){
				if(shoot & mask){
					switch (mask){
					case 1:
						actuator.actuateShooter(1);
						break;
					case 2:
						actuator.actuateShooter(2);
						break;
					case 4:
						actuator.actuateShooter(3);
						break;
					case 8:
						actuator.actuateShooter(4);
						break;
					}
				}
			}
		break;
      case SWEEPER:
        //sweeper
		sweep.updateTargetPos(buf[2]);
        break;
      case STEPPER:
        //stepper
        // stepper_obj.moveStepper(buf[3]);
        stepper.enable = 1;
        stepper.target = buf[3];
			default:;
		}			
	}

  // disable stepper 
  if (buf[0] & STEPPER == 0) {   
    stepper.enable = 0;
  }

}

void moveMotor (void) {
    //sweeper
    //compute difference between desire and current values
    //diff = (round)(IMU_Ratio_Constant * imu.correctError() + 1525);
	//plus or minus depends on how is the shooter platform mounted
    //update sweeper position
	sweep.updateCurrPos(sweep.currTarget() - imu.calculateError());
    //stepper
    stepper.moveStepper();
}

void sendCheckSum(const uint8_t* buf){
    uint8_t checkSum;
    uint8_t local_buf[8] = {0};
    
    //compute checksum
	checkSum = buf[0] ^ buf[1] ^ buf[2] ^ buf[3];
    
    //send back checksum
	CAN.setupCANFrame(local_buf, 4, 1, checkSum);
	CAN.sendMsgBuf(CAN_manipulator, 0, 0, 5, local_buf);
}
