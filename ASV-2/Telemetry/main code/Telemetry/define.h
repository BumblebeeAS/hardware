#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 54

//CONTROL MODE
#define AUTONOMOUS 1
#define MANUAL_RC 2
#define MANUAL_OCS 3
#define STATION_KEEP 4

//SCREEN
#define SCREEN_INT 25
#define SCREEN_CS 22
#define SCREEN_RESET 24
#define OFFSET 40

#define SCREEN_LOOP 1000

//FRISKY
#define RC_INT 19
#define RSSI_THRESHOLD 38

#define FRISKY_FORWARD 2
#define FRISKY_SIDE 1
#define FRISKY_YAW 3
#define FRISKY_ARM 4
#define FRISKY_RSSI 5

#define I2C_ADDR_DAC 0x4C

//OCS
#define XBEE_BAUDRATE 115200
#define N2420_BAUDRATE 9600
#define START_BYTE 0xFE

//Internal stats
#define INT_STAT_COUNT 6

#define INT_PRESS 0
#define HUMIDITY 1
#define CPU_TEMP 2
#define POSB_TEMP 3
#define RSSI_OCS 4
#define RSSI_RC 5

//Power stats
#define POWER_STAT_COUNT 6

#define BATT1_CAPACITY 0
#define BATT2_CAPACITY 1
#define BATT1_CURRENT 2
#define BATT2_CURRENT 3
#define BATT1_VOLTAGE 4
#define BATT2_VOLTAGE 5

//Heartbeat
#define HB_COUNT 13
#define BATT1 9
#define BATT2 10
#define ESC1 11
#define ESC2 12

//TIMEOUTS
#define HB_TIMEOUT 3000
#define HEARTBEAT_LOOP 500
#define THRUSTER_TIMEOUT 100
#define COMMLINK_TIMEOUT 4000
#define FAILSAFE_TIMEOUT 3000
#define STAT_TIMEOUT 2000

//XBEE
#define MAXSIZE 30
#define KILL_SEND 0x416B976D
#define KILL_RCV 0x416B9775
#define OCS_EXT 0x416B969E
#define ASV_EXT 0x416B9783
#define SPARE1 0x416B96A3

#endif // _DEFINES_H
