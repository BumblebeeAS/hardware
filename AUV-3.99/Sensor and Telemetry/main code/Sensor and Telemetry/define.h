#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 54

//SCREEN
#define SCREEN_INT 25
#define SCREEN_CS 22
#define SCREEN_RESET 24

#define SCREEN_LOOP 1000


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


#endif // _DEFINES_H
