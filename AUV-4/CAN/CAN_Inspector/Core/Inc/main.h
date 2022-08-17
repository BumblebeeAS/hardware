#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "can_lib.h"
#include "peripheral_Init.h"
#include "retarget.h"
#include <stdio.h>
#include "Error_Define.h"



#define START_BYTE 254		// (uint8_t) 254 ==> (int8_t) -2
#define END_BYTE 255

#define CAN_PCB_STAT 3


typedef struct SBC_recvMsgBuf_Type {	//ping-pong msg buffer
	uint8_t SBC_recvMsgBuf1[32];
	uint8_t SBC_recvMsgBuf2[32];
} SBC_recvMsgBuf_Type;

uint32_t Error_Status;	//all error reports are described here
							//Initialised in system_Begin();



void Enable_TIM17(void);
void SystemClock_Config(void);
void System_Begin();

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void Error_Handler();

void CAN_SetAllFilters();


void CAN_Log(uint32_t RxFifo);
void CAN_PrintTrafficLog();
void CAN_GenerateReport();
uint8_t CAN_FreqDiagnostic(uint8_t id, uint8_t freq);




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
