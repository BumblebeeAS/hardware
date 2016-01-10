#ifndef _DEFINES_H
#define _DEFINES_H

#define LED_Red_1 		9 
#define LED_Blue_1 		7
#define LED_Green_1 	11
#define LED_Red_2 		10
#define LED_Blue_2		4
#define LED_Green_2		12

#define ADS_ADD_INT 73 // Sense 2
#define ADS_ADD_EXT	72 // Sense 3 Connect external to this.

#define ATM 99974 //Pascals or 14.5PSI
#define PSI100 689475
#define PSI30 206842
#define INTPRES_REF 5
#define LPF_CONSTANT 0.7
#define PRESSURE_TYPE_ABSOLUTE_100 0
#define PRESSURE_TYPE_GAUGE_30 1
#define WaterPin1 A0 // green
#define WaterPin2 A1 // yellow
#define WaterPin3 A2 // orange
#define TempHumAddr 0x27 // V+
#define TempAddr2 0x4A	// SDA
#define TempAddr3 0x4B	// SCL
#define ADC_16 0x48 //
#define NORMAL 0
#define DEBUG_INFO 1
#define LCD_ON 1
#define LCD_OFF 0

#define MANI_1 30
#define MANI_2 31
#define MANI_3 32
#define MANI_4 33
#define MANI_5 34
#define MANI_6 41
#define MANI_7 40
#define MANI_8 37
#define MANI_9 36

#define CAN_Chip_Select 8

#endif // _DEFINES_H
