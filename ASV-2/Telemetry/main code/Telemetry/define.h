#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 54

//CONTROL MODE
#define AUTONOMOUS 0
#define MANUAL_OCS 1
#define MANUAL_RC 2
#define STATION_KEEP 3

//SCREEN
#define SCREEN_INT 25
#define SCREEN_CS 22
#define SCREEN_RESET 24

#define SD_CS 10

//FRISKY
#define RC_INT 19
#define RSSI_THRESHOLD 50

#define FRISKY_FORWARD 2
#define FRISKY_SIDE 1
#define FRISKY_YAW 3
#define FRISKY_ARM 4
#define FRISKY_RSSI 5

#define I2C_ADDR_DAC 0x4C

//OCS
#define XBEE_BAUDRATE 9600

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
#define HB_TIMEOUT 500
#define HB_COUNT 11
#define SBC 0
#define POKB 1
#define POSB 2
#define POPB 3
#define LARS 4
#define OCS 5
#define RC 6
#define BATT1 7
#define BATT2 8
#define ESC1 9
#define ESC2 10

//TIMEOUTS
#define HEARTBEAT_TIMEOUT 500
#define THRUSTER_TIMEOUT 100
#define COMMLINK_TIMEOUT 4000

#endif // _DEFINES_H
