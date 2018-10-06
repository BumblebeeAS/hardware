/*
 * define.h
 *
 * Created: 28 Sep 2018 15:27:44
 *  Author: alanc
 */ 


#ifndef DEFINE_H_
#define DEFINE_H_

//Timer interval definitions
#define SERIAL_STATUS_INTERVAL 1000

///Manipulator definitions
#define GRIP_PIN	6

//CAN Definitions
#define CAN_Chip_Select 8
#define HEARTBEAT_TIMEOUT 500

//CAN commands mapping for manipulation
#define CLOSE_GRIPPER	0x00
#define OPEN_GRIPPER	0x01

#endif /* DEFINE_H_ */