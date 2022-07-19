#ifndef _DEFINES_H
#define _DEFINES_H

//Timer interval definitions
#define DROPPER_INTERVAL 1500  
#define TORPEDO_INTERVAL 400
#define SERIAL_STATUS_INTERVAL 1000
#define DROPPER_STEP 1000

//Actuation pinout
#define DROP A2 // Map to A2GPIO
#define TORP A3 // Map to A3 GPIO
#define stepPin A0 // Map to A0 GPIO
#define dirPin A1 // Map to A1 GPIO

//Grabber Stepper Uart pin
#define SW_RX 6
#define SW_TX 7
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00

//Grabber stepper Stall definitions
#define gSpeed    5000  
#define gTime     1900
//#define gSpeed    4660  //motor velocity in +-(2^23)-1 [μsteps / t]
//#define gTime     2000  //motor run time 3.4 rnds -> 1.5s
#define STALL_VALUE_CLOSE     240 //tune with load
#define STALL_VALUE_OPEN      240

// Grabber stepper definitions
#define microstep 8
#define stepperdelay 150
#define stepsPerRevolution 200

//Servo angle definitions
#define DROP_RESET 10     // 
#define DROP_FIRE 173      //  
#define TORP_RESET 95       // Angle
#define TOP_FIRE 70
#define BOT_FIRE 120


//CAN Definitions
#define CAN_Chip_Select 8
#define HEARTBEAT_TIMEOUT 500

//CAN Heartbeat
#define CAN_ACT_HEARTBEAT 0x0A

//CAN commands mapping for manipulation
#define FIRE_TOP_TORPEDO 0x04
#define FIRE_BOT_TORPEDO 0x10
#define FIRE_DROPPER 0x40
#define ACTIVATE_GRABBER 0x01
#define RELEASE_GRABBER 0x02
#define BOTTLE_GRABBER 0x20

#endif // _DEFINES_H
