#ifndef PMB_DEFINES_H
#define PMB_DEFINES_H

//these are the correct ones

#define MAIN_LOOP_INTERVAL 			100
#define MSG_LOOP_INTERVAL 			500
#define CAN_STATUS_LOOP_INTERVAL 	1000

// #define MAIN_LOOP_INTERVAL 			1000
// #define MSG_LOOP_INTERVAL 			2000
// #define CAN_STATUS_LOOP_INTERVAL 	5000

#define PIN_POWEROFF_SIGNAL			5				
#define PIN_PMB_POWER				3
#define PIN_VEHICLE_POWER			6

//PLEASE MAKE IT ODD
#define MEDIAN_FILTER_SIZE			5
		
#define CELLS						6
		
#define CONFIG_TEMP_SENS			96

#define ADDR_TEMP_SENS				0X4E
#define ADDR_ADC					0x48
#define ADDR_EEPROM0				0X50
#define ADDR_EEPROM1				0X51
#define ADDR_OLED					0X3D

//dummy pin to initialise the OLED ,actually connected to the MCU ground
#define PIN_OLED_RESET				7
		
#define PIN_CAN_SS					8

#define CHANNEL_CURRENT_SENS		2
#define CHANNEL_PRESSURE			1

#define INTPRES_REF 				5

#define DISPLAY_RESET 				4

#endif