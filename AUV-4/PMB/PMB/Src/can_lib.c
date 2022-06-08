#include "can_lib.h"

/*#############Public############*/

void CAN_Begin(uint32_t Mode){
	// configure CAN and put it into initialisation mode
	hcan.Init.Mode = Mode;	//set can mode

	CAN_Init();
	// configure CAN interrupts
	/*
	if (HAL_CAN_ActivateNotification(&hcan,
			CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_FULL
			| CAN_IT_RX_FIFO0_OVERRUN | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO1_FULL
			| CAN_IT_RX_FIFO1_OVERRUN | CAN_IT_ERROR) != HAL_OK)
	{
		Error_Handler();
	}
	*/
	// set default filter
	CAN_InitFilter();
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_READY)
	{	// check CAN status
		// error_status |= 1<< CAN_INIT_ERROR;
		Error_Handler();
	}

	// start CAN node
	// This function changes CAN_state to HAL_CAN_STATE_LISTENING
	HAL_CAN_Start(&hcan);
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_LISTENING)
	{	// check CAN status
		  // error_status |= 1<< CAN_START_ERROR;
		Error_Handler();
	}
	hcan.Instance->MCR &= ~(1<<16);	//This line is needed for CAN to work in debug mode!!!!!!!
	 // NVIC
	 // HAL_NVIC_SetPriority(30,2,0);// CEC_CAN_IRQn -> 30 , priority level ->2
	// enable CEC_CAN_IRQ
	// Put this at last to not let IRQ disrupt printf(&thus hanging Uart)
	// HAL_NVIC_EnableIRQ(30);
}


void CAN_SendMsg(uint32_t id,  uint8_t* Msg, uint8_t len)
{
	uint32_t TxMailbox;
	// configure TxHeader
	CAN_Config_TxHeader(id, len);
	// send msg
	if(HAL_CAN_AddTxMessage(&hcan,&TxHeader, Msg, &TxMailbox )!= HAL_OK){
		// error_status |= 1 << CAN_SENDMSG_ERROR;
		Error_Handler();
	}
}

uint32_t CAN_RecvMsg(uint32_t RxFifo, uint8_t* recvMsg)
{
	uint32_t RxFifo_level;
	RxFifo_level = HAL_CAN_GetRxFifoFillLevel(&hcan, RxFifo);
	if (RxFifo_level == 0){
		return 10;	//No msg is found
	}	else	{

		// receive msg from RxFifo
		if (HAL_CAN_GetRxMessage(&hcan, RxFifo, &RxHeader, recvMsg) == HAL_ERROR){
		}
	}
	return HAL_CAN_GetRxFifoFillLevel(&hcan, RxFifo);
}




void CAN_SetFilter(uint32_t id1, uint32_t Mask1, uint32_t id2, uint32_t Mask2, uint32_t Bank, uint32_t RxFifo)
{
   CAN_FilterTypeDef FilterConfig;
   FilterConfig.FilterIdHigh = id1 << 5;		//set first filter id	//First 11 MSB bits of the 16 bit register, shift 5 times
   FilterConfig.FilterIdLow = id2 << 5;				//set second filter id	//First 11 MSB bits of the 16 bit register, shift 5 times
   FilterConfig.FilterMaskIdHigh = Mask1 << 5;		//set first filter mask	//First 11 MSB bits of the 16 bit register, shift 5 times
   FilterConfig.FilterMaskIdLow = Mask2 << 5;		//set second filter		//First 11 MSB bits of the 16 bit register, shift 5 times
   FilterConfig.FilterFIFOAssignment = RxFifo;
   FilterConfig.FilterBank = Bank;
   FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;		//mask mode
   FilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;	//stdid mode
   FilterConfig.FilterActivation = CAN_FILTER_ENABLE;	//enable filter
   if ( HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK)
   {
	   // error_status |= 1 << CAN_SETFILTER_ERROR;
	 Error_Handler();
   }

}




uint32_t CAN_CheckReceive()
{
	uint32_t RxFifo_level;
	RxFifo_level = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
	if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO1) > RxFifo_level)
	{
		return CAN_RX_FIFO1;
	}
	if (RxFifo_level == 0 ){
		return 10;
	}
	return CAN_RX_FIFO0;
}

uint32_t CAN_GetId(){
	return RxHeader.StdId;
}

void CAN_SetMsgFrame(int8_t TxMsg[], uint8_t start_pos, uint8_t len, int32_t val)
{
	uint8_t i;
	uint8_t required_bytes;  	// required bytes = abs(data) // 128 + 1
	if (val < 0){
		required_bytes = (val ^ -1) + 1;	// flip sign. 2's complement
	}
	required_bytes = required_bytes / 128 + 2;	// number of bytes + 1 (upper bound of for-loop)

	for (i = 0; i < required_bytes ; i++){
		// LSB in the lower pos, MSB in the upper pos
		TxMsg[i+start_pos] = val >> (8 * i);
	}

}

int32_t CAN_ParseMsgFrame(int8_t RxMsg[], uint8_t start_pos, uint8_t stop_pos, uint8_t len)
{
	int32_t val = 0;
	uint8_t i;
	for ( i = 0 ; i < stop_pos-start_pos ; i ++){
		val += RxMsg[i+start_pos] << 8 * i;
	}
	return val;
}

void CAN_PrintMsgFrame(int8_t Msg[],uint8_t len)
{
	printf("\r\n========= CAN_PrintMsgFrame=========\r\n");
	uint8_t i = 0;
	for ( i = 0 ; i < len ; i++){
		printf("%d\t",Msg[i]);
	}
	printf("\r\n");
}

// where are all the vars declared

//void CAN_UpdateError()
//{
//	  uint32_t error;
//	  error = hcan.Instance->ESR;	// monitor this register to see error report
//	  error = (error >> 4) & 0x07; 	// Extract bit 4-6
//	  switch (error)
//	  {
//	  case 1:
//		  Error_Status |= 1 << CAN_STUFF_ERROR;
//		  break;
//	  case 2:
//		  Error_Status |= 1 << CAN_FORM_ERROR;
//		  break;
//	  case 3:
//		  Error_Status |= 1 << CAN_ACKNOWLEDGMENT_ERROR;
//		  break;
//	  case 4:
//		  Error_Status |= 1 << CAN_BIT_RECESSIVE_ERROR;
//		  break;
//	  case 5:
//		  Error_Status |= 1 << CAN_BIT_DOMINANT_ERROR;
//		  break;
//	  case 6:
//		  Error_Status |= 1 << CAN_CRC_ERROR;
//		  break;
//	  case 7:
//		  Error_Status |= 1 << CAN_SOFTWARE_ERROR;
//		  break;
//	  default:
//		  break;
//	  }
//}






/*#############Private############*/

// initialise CAN
void CAN_Init(void)
{
	//this setting work for HSI (8Mhz) only
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 3;
  //hcan.Init.Mode = CAN_MODE;	//This line now become a parameter for CAN_Begin function
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan) != HAL_OK)
  {
	 // error_status |= 1<< CAN_INIT_ERROR;
    Error_Handler();
  }
}


//Initialise filter during initialising CAN
//Accept all msg
void CAN_InitFilter(void)
{
   CAN_FilterTypeDef FilterConfig;

   FilterConfig.FilterIdHigh = 0; // eg: 16 << 5;	//16, First 11 MSB bits of the 16 bit register
   FilterConfig.FilterIdLow = 0; // eg: 17 << 5;	//17
   FilterConfig.FilterMaskIdHigh = 0;  // eg: 0x7FF << 5; All bits must match
   FilterConfig.FilterMaskIdLow = 0;  //eg: 0x7FF << 5;	All bits must match
   FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
   FilterConfig.FilterBank = 12;
   FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
   FilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
   FilterConfig.FilterActivation = CAN_FILTER_DISABLE;
   if ( HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK)
   {
	   // error_status |= 1<< CAN_FILTER_INIT_ERROR;
	   Error_Handler();
   }
   FilterConfig.FilterIdHigh = 0;
   FilterConfig.FilterIdLow = 0;
   FilterConfig.FilterMaskIdHigh = 0;
   FilterConfig.FilterMaskIdLow = 0;
   FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;
   FilterConfig.FilterBank = 13;
   FilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
   FilterConfig.FilterActivation = CAN_FILTER_DISABLE;
   if ( HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK)
   {
	   // error_status |= 1<< CAN_FILTER_INIT_ERROR;
	   Error_Handler();
   }


}

void CAN_Config_TxHeader(uint32_t id, uint32_t len)
{
	// configure CAN header
	TxHeader.StdId = id;
	TxHeader.ExtId = 0;
	TxHeader.DLC = len;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	// recv_databuf is library-wide data buffer
	uint32_t fill_level;
	fill_level = CAN_RecvMsg(CAN_RX_FIFO0, recv_databuf);

	//test send
	//uint8_t data_send[8] = {0x31,0x32,0x33,0x34,0x31,0x32,0x33,0x34};
    //CAN_SendMsg(0,8,data_send);	//id, len, data buf

}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	// recv_databuf is library-wide data buffer
	uint32_t fill_level;
	fill_level = CAN_RecvMsg(CAN_RX_FIFO1, recv_databuf);

	//Test send
	//uint8_t data_send[8] = {0x31,0x32,0x33,0x34,0x31,0x32,0x33,0x34};
    //CAN_SendMsg(0,8,data_send);	//id, len, data buf

}


void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan){
//#ifdef _VERBOSE
	printf("RxFifo0 is full! Hang the process..");
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 , GPIO_PIN_SET);
//#endif
	while(1);
}

void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan){
//#ifdef _VERBOSE
	printf("RxFifo1 is full! Hang the process..");
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 , GPIO_PIN_SET);
//#endif
	while(1);
}



__weak void Error_Handler()
{
	while(1);
}





