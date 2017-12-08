#ifndef _DEFINES_H
#define _DEFINES_H

//SCREEN
#define SCREEN_INT 3
#define SCREEN_CS 7
#define SCREEN_RESET 9

#define SD_CS 10

//Internal stats
#define HUMIDITY 0
#define INT_PRESS 1
#define PMB1_PRESS 2
#define PMB2_PRESS 3
#define BP_TEMP 4
#define SA_TEMP 5
#define TH_TEMP1 6
#define TH_TEMP2 7
#define PMB1_TEMP 8
#define PMB2_TEMP 9
#define CPU_TEMP 10

//Power stats
#define BATT1_CAPACITY 0
#define BATT1_CURRENT 1
#define BATT1_VOLTAGE 2
#define BATT2_CAPACITY 3
#define BATT2_CURRENT 4
#define BATT2_VOLTAGE 5

//Heartbeat
#define SA 1
#define THRUSTER 2
#define TELEMETRY 3
#define BACKPLANE 4
#define SBC_CAN 5
#define SBC 6
#define PMB1 7
#define PMB2 8

/*
Navigation stats:
- IMU
- DVL
- Depth

Heartbeat
*/

#define CAN_Chip_Select 8
#endif // _DEFINES_H
