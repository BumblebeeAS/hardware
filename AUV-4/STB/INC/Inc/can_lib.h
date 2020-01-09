// CAN library V1.2
// Default Oscillator is HSE 32MHz. Change Bittime if other system clock are used



// CAN library V1.1
// add pull up to CAN_TX & CAN_RX to prevent pins floating and fail initialisation

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
// 4. Use CAN_SetMsgFrame() to set msg and use Can_sendMsg(id, datalen, Msgbuf) to publish a can message
// 5. Use CAN_CheckReceive() to get RecvFIFO that contains CAN msg and use CAN_RecvMsg(CAN_RX_FIFO0, Msg) to receive a can message


// How to use retarget
// 1. copy past retarget.c and .h into the respective folder
// 2. exclude syscalls.c from build
// 3. use printf as normal

// CAN LOOPBACK MODE STOPS WORKING WHEN THE BOARD IS PLUGGED ONTO PROTOBORAD!!!!!!!
// update: Set CAN_TX CAN_RX in pull up mode



// How negative numbers are assembled in MSB & LSB form:
// eg: -9999(10) -> 11011000 11110001 (2's)
// 	   Hence MSB -> 11011000(2's, as MSB == 1)  -> -40(10)
//		     LSB -> 11110001(2's, as MSB == 1)  -> -15(10)
// To reassemble MSB & LSB data:
//	   Change printed dec number back to binary (either binary or 2's,depending on the signedness)
//	   Concatenate MSB(2/2's) & LSB (2/2's) and convert back to dec
//


#ifndef __CAN_LIB_H
#define __CAN_LIB_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_can.h"		//can library
#include "stm32f0xx_hal_cortex.h"	//nvic
#include <stdio.h>				//printf


 /*#############Configuration############*/
//#define _VERBOSE	1
#define CAN_MODE	CAN_MODE_NORMAL	//CAN_MODE_NORMAL  // CAN_MODE_LOOPBACK


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
// the lower level CAN_AddTxMsg accept uint8_t data array, possible problems(?)
void CAN_SendMsg(uint32_t id, uint8_t* Msg,uint8_t len);


// note that the Msg goes into a FiFo according to the filter bank configuration
// hence heartbeat can be configured to go only into FIFO2 while rest of the data goes only into FIFO1
// Return the number of pending messages left in the selected FIFO
// Return value = 10 if no msg is found
uint32_t CAN_RecvMsg(uint32_t RxFIFO, uint8_t* recvMsg);


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


/**
  * @brief  Check which RxFifo contains msg
  * @retval The RxFifo that has more pending msgs (RxFifo 0 take precedence if fill level are same)
  * 		Return value = 10 if no msg is found
  */
uint32_t CAN_CheckReceive();


/**
  * @brief  Get ID from the msg. Use this function after CAN_RecvMsg() call
  * @retval The RxFifo that has more pending msgs (RxFifo 0 take precedence if fill level are same)
  * 		Return value = 10 if no msg is found
  */
uint32_t CAN_GetId();


/**
  * @brief  Setup a Tx Can Frame.
  * 		Can Frame accepts up to 8 bytes of data
  * 		If the data is outside of range -127 < data < 127, len > 1 and data is sent
  * 		in MSB ( in the upper array pos) and LSB ( in the lower array pos)
  * @param1 TxMsg[]	Msg buffer to be sent
  * 				array size must be no more than 8
  * @param2 start_pos 	start position of where data is to be written
  * 				0 - 7
  * @param3 len		Full Length of the Msg
  * 				1 - 8
  * @param4 val 	value to be saved to the Msg buffer
  * @retval None
  */

void CAN_SetMsgFrame(int8_t TxMsg[], uint8_t start_pos, uint8_t len, int32_t val);


/**
  * @brief  Parse a Rx Can Frame.
  * 		Automatically reassembles MSB & LSB if data is more than 1 byrte
  * @param1 RxMsg[]	Msg buffer received
  * 				Msg is received from CAN_RecvMsg() function
  * @param2 start_pos 	start position of where data starts to read
  * 					0 - 7
  * @param3 stop_pos	stop position of where data stops to read. **EXCLUSIVE**
  * @param3 len			Full Length of the Msg
  * 					1 - 8
  * @retval Parsed Msg
  */
int32_t CAN_ParseMsgFrame(int8_t RxMsg[], uint8_t start_pos, uint8_t stop_pos , uint8_t len);


void CAN_PrintMsgFrame(int8_t Msg[],uint8_t len);


//Check error register and output to global Error_Status variable
void CAN_UpdateError();


#ifdef __cplusplus
}
#endif

#endif


