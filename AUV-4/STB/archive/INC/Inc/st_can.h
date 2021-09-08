

#ifndef _ST_CAN_H
#define _ST_CAN_H


#ifdef __cplusplus
 extern "C" {
#endif


#include "main.h"
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
#define CAN_ALL_CONFIG 0xA
#define CAN_STB_SENS 0xB
#define CAN_STB_UP 0xC
#define CAN_STB_GYRO 0xD
#define CAN_STB_ACC 0xE
#define CAN_STB_MAG 0xF
#define CAN_STB_STAT 0x10
#define CAN_THT_STAT_1 0x11
#define CAN_THT_STAT_2 0x12
#define CAN_THT_PWM_1 0x13
#define CAN_THT_PWM_2 0x14
#define CAN_ACT_STAT 0x15
#define CAN_TAB_STAT 0x16
#define CAN_BATT1_STAT 0x17
#define CAN_PMB1_STAT 0x18
#define CAN_BATT2_STAT 0x19
#define CAN_PMB2_STAT 0x1A
#define CAN_TAB_ERROR 0x20
#define CAN_STB_ERROR 0x21
#define CAN_PMB1_ERROR 0x22
#define CAN_PMB2_ERROR 0x23
#define CAN_SBCCAN_ERROR 0x24

//CAN Heartbeat
#define HEARTBEAT_SBC 0x01
#define HEARTBEAT_SBC_CAN 0x02
#define HEARTBEAT_PCB 0x03
#define HEARTBEAT_Thruster_Board 0x04
#define HEARTBEAT_STB 0x05
#define HEARTBEAT_Manipulator 0x06
#define HEARTBEAT_PMB1 0x07
#define HEARTBEAT_PMB2 0x08

 void publishCAN_message(void);

 void publishCAN_heartbeat(void);

 void publishCAN_uptime(void);

 void update_can(void);

#endif
