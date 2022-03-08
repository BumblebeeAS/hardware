#ifndef _DEFINES_H
#define _DEFINES_H

//Timer interval definitions
#define DROPPER_INTERVAL 500
#define TORPEDO_INTERVAL 400
#define SERIAL_STATUS_INTERVAL 1000

//Manipulator definitions
#define DROP 16 // Map to 16 GPIO
#define TORP 17 // Map to 17 GPIO
#define stepPin 14 // Map to 14 GPIO
#define dirPin 15 // Map to 15 GPIO

#define stepsPerRevolution 200

//CAN Definitions
#define CAN_Chip_Select 8
#define HEARTBEAT_TIMEOUT 500

//CAN Heartbeat
#define CAN_ACT_HEARTBEAT 0x06

//CAN commands mapping for manipulation
#define FIRE_TOP_TORPEDO 0x04
#define FIRE_BOT_TORPEDO 0x10
#define FIRE_DROPPER 0x40
#define ACTIVATE_GRABBER 0x01
#define RELEASE_GRABBER 0x02

#endif // _DEFINES_H
