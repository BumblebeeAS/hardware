// Stepper Position:
// // Stepper 0 & 3 : long distance, moving out/forward when opening 
// // Stepper 1 & 2 : short distance, moving in/backward when opening
//
#ifndef _DEFINES_H
#define _DEFINES_H

#define DEBUG 

//---- CAN definitions ---------------------
#define CAN_CS_CHIP 8
#define CAN_DTLS_COMMAND 0x15           // id for can dtls receive
#define CAN_DTLS_STATS 0x16             // id for can dtls send 
#define FINISH_CLOSING 1
#define FINISH_OPENING 2
#define FAULTY_STEPPER 3
#define CLOSE 1
#define OPEN 2
#define CAN_HEARTBEAT 0x03              // id for can heartbeat msg
#define DTLS_HEARTBEAT 0x05             // value for dtls heartbeat sent to can
#define CAN_HEARTBEAT_INTERVAL  500     // heart beat publish at 2Hz
#define ACOUS_EXTEND 1
#define ACOUS_RETRACT 2

//---- hydrophone actuation definitions -------
#define ACOUS_EXTEND 1
#define ACOUS_RETRACT 2
#define ACOUS_DIR_PIN 9
#define ACOUS_PWM_PIN 10
#define ACOUS_DELAY 10000

//---- tmc2209 definitions---------------------
#define SW_RX 6 //sw serial for uart line 1
#define SW_TX 7
#define R_SENSE 0.11f
#define EN 5

#define microstep 16 
#define gSpeed 4000
// #define gTime 2000 //no more cause it runs infinitely
#define STALL_VALUE 330

#define FORWARD 0
#define BACKWARD 1

//-----------------------------------

#endif
