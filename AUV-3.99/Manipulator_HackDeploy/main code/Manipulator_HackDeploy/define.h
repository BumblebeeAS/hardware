#ifndef _DEFINES_H
#define _DEFINES_H

//Timer interval definitions
#define DROPPER_INTERVAL 500
#define TORPEDO_INTERVAL 400
#define SERIAL_STATUS_INTERVAL 1000

///Manipulator definitions
#define MANI_1 30
#define MANI_2 31 // Map to grabber
#define MANI_3 32 // Map to dropper
#define MANI_4 33 // Map to top torpedo
#define MANI_5 34 // Map to bot torpedo
#define MANI_6 41 
#define MANI_7 40
#define MANI_8 37
#define MANI_9 36
#define TORP1 22 // Map to 22 GPIO
#define TORP2 23 // Map to 23 GPIO


//CAN Definitions
#define CAN_Chip_Select 8
#define HEARTBEAT_TIMEOUT 500


//CAN commands mapping for manipulation
#define RETRACT_GRABBER 0x01
#define EXTEND_GRABBER 0x02
#define FIRE_TOP_TORPEDO 0x04
#define FIRE_BOT_TORPEDO 0x10
#define FIRE_DROPPER 0x40


#endif // _DEFINES_H
