//===========================================
//
//    DEFINES FOR ASV 3 TElEMETRY
//
//===========================================

#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 54

//CONTROL MODE
#define AUTONOMOUS 1
#define MANUAL_FRSKY 2
#define MANUAL_OCS 3
#define STATION_KEEP 4

// screen
#define SCREEN_INT 25
#define SCREEN_CS 22
#define SCREEN_RESET 24
#define OFFSET 40

//FRISKY
#define RC_INT 19
#define RSSI_THRESHOLD 38

#define FRISKY_SIDE 1           // Channel 2 on Frsky
#define FRISKY_FORWARD 2
#define FRISKY_YAW 3
#define FRISKY_ARM 4
#define FRISKY_RSSI 5
#define FRISKY_KILL 6

//Internal stats
#define INT_STAT_COUNT 7

#define INT_PRESS 0               // mbar 
#define HUMIDITY 1                // RH %
#define CPU_TEMP 2                // Degree Celsius
#define POSB_TEMP 3               // Degree Celsius
#define RSSI_OCS 4                // 1db
#define RSSI_FRSKY 5              // 1db
#define HULL_LEAK 6               // 1 = leak, 0 = no leak 

#define I2C_ADDR_DAC 0x0C         // DAC address

//Power stats
#define POWER_STAT_COUNT 6

#define BATT1_CAPACITY 0          // %
#define BATT2_CAPACITY 1
#define BATT1_CURRENT 2           // mA
#define BATT2_CURRENT 3
#define BATT1_VOLTAGE 4           // mV
#define BATT2_VOLTAGE 5 

//Heartbeat
#define HB_COUNT 17               // HB array starts from index 1
#define POSB 1
#define POPB 2
#define POKB 3
#define TELEMETRY 4
#define DTLS 5
#define SBC 6
#define OCS 7
#define FRSKY 8
#define LOGICBACKPLANE 9
#define BALLSHOOTER 10
#define KILL 11
#define ACTUATED_THRUSTERS 12
#define BATT1 13
#define BATT2 14
#define ESC1 15
#define ESC2 16

// TIMEOUTS
#define HB_TIMEOUT 1000
#define STAT_TIMEOUT 3000
#define FAILSAFE_TIMEOUT 2000

#define TIMEOUT_COUNT 6
#define POSB_TIMEOUT 0
#define OCS_TIMEOUT 1
#define FRSKY_TIMEOUT 2
#define SBC_TIMEOUT 3
#define BATT1_TIMEOUT 4
#define BATT2_TIMEOUT 5

// LOOPS
#define HEARTBEAT_LOOP 500
#define SCREEN_LOOP 1000
#define CONTROLLINK_LOOP 500
#define THRUSTER_LOOP 100


#endif // _DEFINES_H
