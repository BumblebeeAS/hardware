#ifndef _DEFINES_H
#define _DEFINES_H

//Timer interval definitions
#define DROPPER_INTERVAL 500
#define TORPEDO_INTERVAL 400
#define SERIAL_STATUS_INTERVAL 1000

///Manipulator definitions
#define DROP A2 
#define TORP A3 
#define DIR A1
#define STEP A0 
#define STEPSPERREVOLUTION 200
#define STEPPER_DELAY 3500


//CAN Definitions
#define CAN_Chip_Select 8        
#define HEARTBEAT_TIMEOUT 500

//CAN commands mapping for manipulation
#define FIRE_TOP_TORPEDO 0x04
#define FIRE_BOT_TORPEDO 0x10
#define FIRE_DROPPER 0x40
//
#define RELEASE_GRABBER 0X02
#define ACTIVATE_GRABBER 0x01


#endif // _DEFINES_H
