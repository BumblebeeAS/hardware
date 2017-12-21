#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 54

//SCREEN
#define SCREEN_INT 25
#define SCREEN_CS 22
#define SCREEN_RESET 24

#define SD_CS 10

//FRISKY
#define RC_INT 19

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
#define HB_TIMEOUT 5000
#define HB_COUNT 10
#define SBC 0
#define POKB 1
#define POSB 2
#define POPB 3
#define OCS 4
#define RC 5
#define BATT1 6
#define BATT2 7
#define ESC1 8
#define ESC2 9

#endif // _DEFINES_H
