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
#include "retarget.h"				//comment out this if you do not intend to use printf
#include <stdio.h>					//printf


 /*#############Configuration############*/
//#define _VERBOSE	1

 extern CAN_HandleTypeDef hcan;

/*#############Private############*/

CAN_TxHeaderTypeDef TxHeader;		//Node-specific TxHeader
CAN_RxHeaderTypeDef RxHeader; 		//place where the received header will be stored
uint8_t recv_databuf[8];			//place where the received msg will be stored

void CAN_Init(void); 				//initialise CAN
void CAN_InitFilter(void);			//Initialise filter during initialising CAN. Accept all msg
void CAN_Config_TxHeader(uint32_t id, uint32_t len);	//configure Node-specific TxHeader
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan);
void Error_Handler(void);


/*#############Public############*/


/**
  * @brief  Initialise CAN peripheral and supporting peripheral
  * 		Initialise Acceptance Filter
  * 		notification(interrupt) disabled (uncomment notification and NVIC to enable)
  * @param1 Mode	Specify if normal or loopback mode is to be used
  * 				CAN_MODE_NORMAL / CAN_MODE_LOOPBACK
  * @retval None
  */

void CAN_Begin(uint32_t Mode);



/**
  * @brief   send msg
  *			 Standard ID (0 -> 0x7FF ) and Length of msg (0 -> 8)is required
  * 		 Extended ID is not used
  *			 IDE(type of specifier) is set to be in standard id mode internally
  *			 RTR(type of frame) is set to be in data type
  *			 TransmitGlobalTime Function is disabled
  * @param1 id1		CAN_Id
  * 				0 - >0x7FF (11bit stdid)
  * @param2 *Msg 	The array that stores tx message
  * @param3 len		Length of the tx message
  * @retval None
  */
void CAN_SendMsg(uint32_t id, uint8_t* Msg,uint8_t len);


/**
  * @brief   Receive msg from a FIFO
  * 		 You can use CAN_CheckReceive() to get a FIFO that contains pending message
  * @param1 RxFIFO	Specify the FIFO to get message from
  * 				CAN_RX_FIFO0 / CAN_RX_FIFO1
  * @param2 *Msg 	Specify the array which the function will write the received message to
  * @retval Return the number of pending messages left in the selected FIFO
  *  		Return value = 10 if no msg is found
  */
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

// Initially Used for debugging
void CAN_PrintMsgFrame(int8_t Msg[],uint8_t len);


//Check error register and output to global Error_Status variable
// used for SBC-CAN only due to large incompatibility issue
void CAN_UpdateError();


#ifdef __cplusplus
}
#endif

#endif

