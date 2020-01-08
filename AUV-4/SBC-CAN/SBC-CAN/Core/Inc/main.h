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


uint32_t Error_Status;	//all error reports are described here
							//Initialised in system_Begin();



void Enable_Uart_Int();
void Enable_TIM17(void);
void SystemClock_Config(void);
void System_Begin();

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void Error_Handler();

void SBC_ReceptionHandler(uint8_t *recvMsgBuf);
void SBC_ResetBuf(uint8_t *buf);
void Enable_Uart_Int();


void SBC_Routine();
void CAN_Routine();

void CAN_SetAllFilters();
void CAN_ReceptionHandler(uint8_t* recvMsgBuf);
void CAN_PowerCtrl(uint8_t* recvMsgBuf);




#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
