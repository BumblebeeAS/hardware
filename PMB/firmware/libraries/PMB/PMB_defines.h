#ifndef PMB_DEFINES_H
#define PMB_DEFINES_H

#define PMB_DEBUG_MODE				1

// #define MAIN_LOOP_INTERVAL 			100
// #define MSG_LOOP_INTERVAL 			500
// #define CAN_STATUS_LOOP_INTERVAL 	1000

#define MAIN_LOOP_INTERVAL 			1000
#define MSG_LOOP_INTERVAL 			2000
#define CAN_STATUS_LOOP_INTERVAL 	5000

#define PIN_POWEROFF_SIGNAL			5				
#define PIN_PMB_POWER				3
#define PIN_VEHICLE_POWER			6

//PLEASE MAKE IT ODD
#define MEDIAN_FILTER_SIZE			5

#define BATTERY_CAPACITY 			10000
		
#define CELLS						6
		
#define CONFIG_TEMP_SENS			96

#define ADDR_TEMP_SENS				0X4E
#define ADDR_ADC					72
#define ADDR_EEPROM0				0X50
#define ADDR_EEPROM1				0X51
#define ADDR_OLED					0X3D
		
#define PIN_CAN_SS					8

#define CHANNEL_CURRENT_SENS		0
#define CHANNEL_PRESSURE			2

#define INTPRES_REF 				5

#define DISPLAY_RESET 				4

#endif