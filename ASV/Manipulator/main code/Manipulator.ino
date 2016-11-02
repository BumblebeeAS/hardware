#include <can.h>
#include <SPI.h>
#include <Servo.h>
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
Sweeper sweep(UPDATE_INTERVAL);
Actuator actuator;
Step stepper_obj; 

//Time variables
uint32_t heartbeat_loop = 0;

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
  MoveMotor();
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
	CAN.setupCANFrame(buf,0,1,HEARTBEAT_Manipulator);
	CAN.sendMsgBuf(CAN_heartbeat,0,1,buf);
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
				shoot = buf[2]>>3;   // right shift mani_state such that shoot contains only bit for shooter
        //TODO finish shooter part
				break;
      case SWEEPER:
        //sweeper
        sweep.enable = 1;
        sweep.target = buf[2];
        break;
      case STEPPER:
        //stepper
        // stepper_obj.moveStepper(buf[3]);
        stepper_obj.enable = 1;
        stepper_obj.target = buf[3];
			default:;
		}			
	}
  // disable stepper 
  if (buf[0] & STEPPER == 0) {   
    stepper_obj.enable = 0;
  }
  // disable servo
  if (buf[0] & SWEEPER == 0) {
    sweep.enable = 0;
  }
}

void MoveMotor (void) {
  uint8_t diff = 0;
    //sweeper
    //read current IMU values
    imu.readAccTempGyro();
    //compute difference between desire and current values
    // diff = (round)(IMU_Ratio_Constant * imu.correctError() + 1525);
    diff = sweep.target - imu.correctError();  
    // plus or minus depends on how is the shooter platform mounted
    diff = map(diff,0,38,1980,2456);
    //update sweeper position
    sweep.update(diff);
    //stepper
    stepper_obj.moveStepper();
}

void sendCheckSum(const uint8_t* buf){
    uint8_t checkSum;
    uint8_t local_buf[8] = {0};
    
    //compute checksum
    checkSum = buf[0]^buf[1]^buf[2]^buf[3];
    
    //send back checksum
    CAN.setupCANFrame(local_buf,4,1,checkSum);
    CAN.sendMsgBuf(CAN_manipulator,0,0,5,local_buf);
}

