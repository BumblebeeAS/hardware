// CAN library V1
// This library only works with stm32f042

// To port over to other stm32 chips, reconfigure GPIOs is required
// macro definition problems may also occur

// Steps to run CAN
// 1. Configure CAN and put CAN into initialisation mode
// 2. Configure GPIO functions and NVIC
// 3. If Interrupt mode is used, implement CEC_CAN_IRQHandler(void) to be redirect to  HAL_CAN_IRQHandler(&hcan)
//    (In stm32f0xx_it.c)
//	  (Remember to include can_lib.h in stm32f0xx_it.c for isProcessing condition)
// 4. Configure CAN filter
//	  At least one filter bank need to be configured for each Rxfifo to allow msg to be accepted
//	  into that fifo
// 5. Start CAN (HAL_CAN_Start(&hcan))
// 6. Configure TxMailbox
// 7. Turn on notification mode (interrupt mode)
// 8. Implement various Callback functions

// How to use this library
// 1. Prerequisit (stm32f042)
// 2. Run CAN_Begin() function.
//	  It handles CAN peripheral initialisation, MSP initialisation(Clock, GPIO, NVIC),
//	  acceptance filter(default, accept all), activate notification mode(interrupt mode)
//    and activate node.
// 3. Use setFilter to set acceptance filter
// 4. Use Can_sendMsg(id, datalen, Msgbuf) to publish a can message
// 5. Can_Msg reception is handled by Callback functions (triggered by interrupts)


#ifndef __CAN_LIB_H
#define __CAN_LIB_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_can.h"		//can library
#include "stm32f0xx_hal_cortex.h"	//nvic
#include "stdio.h"					//printf
#include "retarget.h"				//printf retarget


/*#############Private############*/

CAN_HandleTypeDef hcan; 			//hcan struct
CAN_TxHeaderTypeDef TxHeader;		//Node-specific TxHeader
CAN_RxHeaderTypeDef RxHeader; 		//place where the received header will be stored
uint8_t recv_databuf[8];			//place where the received header will be stored
volatile uint8_t isProcessing;	//for rejecting an interrupt, see the IRQ handler

void CAN_Init(void); 				//initialise CAN
void CAN_InitFilter(void);			//Initialise filter during initialising CAN. Accept all msg
void CAN_Config_TxHeader(uint32_t id, uint32_t len);	//configure Node-specific TxHeader
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan);

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan);	//initialise CAN supporting peripherals & NVIC
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan);
void HAL_Error_Handler(void);
void CAN_Error_Handler(void);

/*#############Public############*/

// initialise CAN peripheral and supporting peripheral
// initialise Acceptance Filter
// notification(interrupt) disabled (uncomment notification and NVIC to enable)
// if CAN initilisation failed. This could be a reason that bit timing is set wrong.
// Use CAN bit timing calculator to verify precaler, Seg1 and Seg2 Quanta
void CAN_Begin(void);

// send msg
// Standard ID (0 -> 0x7FF ) and Length of msg (0 -> 8)is required
// Extended ID is not used
// IDE(type of specifier) is set to be in standard id mode internally
// RTR(type of frame) is set to be in data type
// Use CAN_requestMsg for remote request
// TransmitGlobalTime Function is disabled
// Can_sendMsg(id, datalen, Msgbuf)
void CAN_SendMsg(uint32_t id, uint8_t Msg[]);



// note that the Msg goes into a FiFo according to the filter bank configuration
// hence heartbeat can be configured to go only into FIFO2 while rest of the data goes only into FIFO1
// Return the number of pending messages left in the selected FIFO
// Return value == 10 indicates no msg is found
uint32_t CAN_RecvMsg(uint32_t RxFIFO, uint8_t recvMsg[]);



/**
  * @brief  Setup Acceptance filter. One filter bank contains 2 filters
  * 		If only one filter is to be used, fill id2 == id1, mask2 == mask1
  * @param1 id1		CAN_Id to the first filter
  * 				0 - >0x7FF (11bit stdid)
  * @param2 Mask1 	Mask to the first filter
  * 				0: Don't care
  * 				1: must match that bit
  * @param3 id2		CAN_Id to the first filter
  * 				0 - >0x7FF (11bit stdid)
  * @param4 Mask2 	Mask to the first filter
  * 				0: Don't care
  * 				1: must match that bit
  * @param5 Bank 	Filter bank. One Bank contains at most 2 filters
  * 				0 -> 13
  * @param6 RxFifo 	Which RxFIFO to store msg
  * 				CAN_RX_FIFO0 / CAN_RX_FIFO1
  * @retval None
  */
void CAN_SetFilter(uint32_t id1, uint32_t Mask1, uint32_t id2, uint32_t Mask2, uint32_t Bank, uint32_t RxFifo);

/*
void CAN_SetMsgFrame;
void CAN_ParseMsgFrame;
*/


#ifdef __cplusplus
}
#endif

#endif

