#ifndef _DEFINES_H
#define _DEFINES_H

//I2C
#define ADS_ADDR 0X48
#define HUMIDITY_ADDR 0X27

//CAN
#define CAN_Chip_Select 8
#define CAN_INT 2

//SCREEN
#define SCREEN_INT 3
#define SCREEN_CS 57
#define SCREEN_RESET 66
#define OFFSET 0 // 40 for ASV2.0 screen

//LED
#define RED 12		//23
#define BLUE 10		//24
#define GREEN 11	//22

//Sensor
#define Vref 5		//MPXH Vdd is the Vref
#define LPF_CONSTANT 0.7
#define ADS_DELAY 5
#define LPF_LOOP 25

//SONAR
#define SONAR_IN 69
#define SONAR_OUT 26

//Internal stats
#define INT_STAT_COUNT 10

#define EXT_PRESS 0
#define INT_PRESS 1
#define PMB1_PRESS 2
#define PMB2_PRESS 3 
#define PMB1_TEMP 4
#define PMB2_TEMP 5
#define CPU_TEMP 6
#define HUMIDITY 7
#define ST_TEMP 8
#define DNA_PRESS 9

//Power stats
#define POWER_STAT_COUNT 6

#define BATT1_CAPACITY 0
#define BATT2_CAPACITY 1
#define BATT1_CURRENT 2
#define BATT2_CURRENT 3
#define BATT1_VOLTAGE 4
#define BATT2_VOLTAGE 5

//Heartbeat
#define HB_COUNT 9

//TIMEOUTS
#define SCREEN_LOOP 1000
#define HB_TIMEOUT 3000
#define HEARTBEAT_LOOP 500
#define STAT_TIMEOUT 2000

#endif // _DEFINES_H
