

#ifndef _ST_CAN_H
#define _ST_CAN_H


#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_spi.h"		//can library
#include "stm32f0xx_hal_cortex.h"	//nvic
#include "stdio.h"					//printf
#include "stm32f0xx_hal.h"
#include "sensor.h"
#include "screen.h"
#include "can_lib.h"

#define CAN_THT_CONTROL_1 0
#define CAN_THT_CONTROL_2 1
#define CAN_ACT_CONTROL 2
#define CAN_PCB_STAT 3
#define CAN_HEARTBEAT 4
#define CAN_CPU_TEMP 5
#define CAN_TAB_CONFIG 6
#define CAN_STB_CONFIG 7
#define CAN_PMB1_CONFIG 8
#define CAN_PMB2_CONFIG 9
#define CAN_ALL_CONFIG 10
#define CAN_STB_SENS 11
#define CAN_STB_UP 12
#define CAN_STB_GYRO 13
#define CAN_STB_ACC 14
#define CAN_STB_MAG 15
#define CAN_STB_STAT 16
#define CAN_THT_STAT_1 17
#define CAN_THT_STAT_2 18
#define CAN_THT_PWM_1 19
#define CAN_THT_PWM_2 20
#define CAN_ACT_STAT 21
#define CAN_TAB_STAT 22
#define CAN_BATT1_STAT 23
#define CAN_PMB1_STAT 24
#define CAN_BATT2_STAT 25
#define CAN_PMB2_STAT 26
#define CAN_TAB_ERROR 32
#define CAN_STB_ERROR 33
#define CAN_PMB1_ERROR 34
#define CAN_PMB2_ERROR 35
#define CAN_SBCCAN_ERROR 36

//CAN Heartbeat
#define HEARTBEAT_SA 0x01
#define HEARTBEAT_Thruster 0x02
#define HEARTBEAT_Backplane 0x03
#define HEARTBEAT_SBC_CAN 0x04
#define HEARTBEAT_SBC 0x05
#define HEARTBEAT_PMB1 0x06
#define HEARTBEAT_PMB2 0x07
#define HEARTBEAT_Telemetry 0x08

 void publishCAN_message(void);

 void publishCAN_heartbeat(void);

 void publishCAN_uptime(void);

 void update_can(void);

#endif
