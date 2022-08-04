#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "can_lib.h"
#include "msg_queue.h"
#include "peripheral_Init.h"
#include "stm32f0xx_hal_iwdg.h"

#include <stdio.h>
#include <stdlib.h>

#define START_BYTE 254		// (uint8_t) 254 ==> (int8_t) -2

msg_queue uart_txbuf, uart_rxbuf;

#define CAN_PCB_STAT 3

void SystemClock_Config(void);
void System_Begin();

void Error_Handler();

void SBC_Routine();
void CAN_Routine();

void CAN_SetAllFilters();
void CAN_PowerCtrl(uint8_t pwr_ctr);
void CAN_SendHeartBeat();
void SBC_SendPowerControlStatus();
void SBC_SendButtonStatus();

// New functions
void UART_RecvStart();
void UART_RecvPause();
void UART_RecvResume();
void UART_TranStart();
void CAN_RecvStart();
void CAN_TranStart();

void UART_WaitIdle(uint32_t timeout);

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
