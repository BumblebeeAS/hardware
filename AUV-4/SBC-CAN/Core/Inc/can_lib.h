// How to use this library
// 1. Run CAN_Begin(Mode) function. You can pass in CAN_MODE_NORMAL or CAN_MODE_LOOPBACK.
//	  It handles CAN peripheral initialisation, MSP initialisation(Clock, GPIO),
//	  acceptance filter(default, accept all), and activate node.
// 3. Use can_SetFilter to set acceptance filter
// 4. Use CAN_SetMsgFrame() to set a CAN msg
// 5. Use Can_SendMsg(id, datalen, Msgbuf) to publish a can message
// 6. Use CAN_CheckReceive() to get a RecvFIFO that contains CAN msg
// 7. Use CAN_RecvMsg(CAN_RX_FIFO0, Msg) to receive a can message from a FIFO
// 8. Use CAN_GetId() after receiving msg to check where it comes from



#ifndef __CAN_LIB_H
#define __CAN_LIB_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_cortex.h"	//nvic
#include <stdio.h>					//printf

void CAN_Init(void); 				//initialise CAN
void CAN_InitFilter(void);			//Initialise filter during initialising CAN. Accept all msg
void CAN_Begin(uint32_t Mode);
void CAN_SetFilter(uint32_t id1, uint32_t Mask1, uint32_t id2, uint32_t Mask2, uint32_t Bank, uint32_t RxFifo);


#ifdef __cplusplus
}
#endif

#endif

