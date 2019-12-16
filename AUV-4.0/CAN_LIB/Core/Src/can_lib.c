#include "can_lib.h"


//#define VERB 1 //verbose mode

/*#############Public############*/

void CAN_Begin(void){
#ifdef VERB
	printf("\r\n========= CAN initialisation =========\r\n");
#endif
	// configure CAN and put it into initialisation mode
	CAN_Init();
	// configure CAN interrupts
	/*
	if (HAL_CAN_ActivateNotification(&hcan,
			CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_FULL
			| CAN_IT_RX_FIFO0_OVERRUN | CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO1_FULL
			| CAN_IT_RX_FIFO1_OVERRUN | CAN_IT_ERROR) != HAL_OK)
	{
		HAL_Error_Handler();
	}
	*/
	// set default filter
	CAN_InitFilter();
#ifdef VERB
	printf("End CAN initialisation...\r\n");
	printf("Check CAN state...\r\n");
#endif
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_READY)
	{	// check CAN status
		CAN_Error_Handler();
	}

#ifdef VERB
	printf("CAN state ready!...\r\n");
	printf("CAN initialized and ready for use!\r\n");
	printf("Activating CAN Node...\r\n");
#endif
	// start CAN node
	// This function changes CAN_state to HAL_CAN_STATE_LISTENING
	isProcessing = 0;
	HAL_CAN_Start(&hcan);
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_LISTENING)
	{	// check CAN status
		CAN_Error_Handler();
	}
#ifdef VERB
	printf("CAN Node Activated!\r\n");
#endif
	 // enable CEC_CAN_IRQ
	// Put this at last to not let IRQ disrupt printf(&thus hanging Uart)
	// HAL_NVIC_EnableIRQ(30);
}


void CAN_SendMsg(uint32_t id,  uint8_t Msg[])
{
	uint32_t TxMailbox;
	uint8_t len = sizeof(Msg);
#ifdef VERB
	printf("\r\n========= CAN_sendMsg =========\r\n");
	printf("Configuring_TxHeader...\r\n");
#endif
	// configure TxHeader
	CAN_Config_TxHeader(id, len);
#ifdef VERB
	printf("Print Msg Sent:\r\n");
	printf("id: %ld\t",id);
	uint8_t i = 0;
	for(i = 0 ; i < len ; i++){
		printf("%c  ", Msg[i]);
	}
	printf("\r\n");
#endif
	// send msg
	if(HAL_CAN_AddTxMessage(&hcan,&TxHeader, Msg, &TxMailbox )!= HAL_OK){
		HAL_Error_Handler();
	}
}

uint32_t CAN_RecvMsg(uint32_t RxFifo, uint8_t recvMsg[])
{
	uint32_t RxFifo_level;
#ifdef VERB
	printf("\r\n========= CAN_recvMsg =========\r\n");
	printf("Checking RxFifo %lu...\r\n",RxFifo);
#endif
	RxFifo_level = HAL_CAN_GetRxFifoFillLevel(&hcan, RxFifo);
	if (RxFifo_level == 99){
#ifdef VERB
	printf("No Msg found in RxFifo %lu...\r\n",RxFifo);
#endif
		return 10;	//No msg is found
	}	else	{
#ifdef VERB
	printf("Receiving Msg from RxFifo %lu...\r\n",RxFifo);
#endif
		// receive msg from RxFifo
		if (HAL_CAN_GetRxMessage(&hcan, RxFifo, &RxHeader, recvMsg) == HAL_ERROR){
			HAL_Error_Handler();
		}
#ifdef VERB
	printf("Print Msg Received:\r\n");
	printf("id: %lu\t",RxHeader.StdId);
	uint8_t i = 0;
	for(i = 0 ; i < RxHeader.DLC ; i++){
		printf("%lu  ", recvMsg[i]);
	}
	printf("\r\n");
#endif
	}
	return HAL_CAN_GetRxFifoFillLevel(&hcan, RxFifo);
}





void CAN_SetFilter(uint32_t id1, uint32_t Mask1, uint32_t id2, uint32_t Mask2, uint32_t Bank, uint32_t RxFifo)
{
#ifdef VERB
	printf("\r\n========= CAN_SetFilter=========\r\n");
#endif
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
	 HAL_Error_Handler();
   }
#ifdef VERB
	printf("Mask1: %lu\r\n",Mask1);
	printf("id1: %lu\r\n",id1);
	printf("Mask1: %lu\r\n",Mask2);
	printf("id1: %lu\r\n",id2);
	printf("Bank: %lu\r\n",Bank);
	printf("RxFifo: %lu\r\n",RxFifo);
#endif
}










/*#############Private############*/

// initialise CAN
void CAN_Init(void)
{
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 1;
  hcan.Init.Mode = CAN_MODE_NORMAL ;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan) != HAL_OK)
  {
    HAL_Error_Handler();
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
   FilterConfig.FilterBank = 0;
   FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
   FilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
   FilterConfig.FilterActivation = CAN_FILTER_ENABLE;
   if ( HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK)
   {
     HAL_Error_Handler();
   }
   FilterConfig.FilterIdHigh = 0;
   FilterConfig.FilterIdLow = 0;
   FilterConfig.FilterMaskIdHigh = 0;
   FilterConfig.FilterMaskIdLow = 0;
   FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;
   FilterConfig.FilterBank = 1;
   FilterConfig.FilterActivation = CAN_FILTER_ENABLE;
   if ( HAL_CAN_ConfigFilter(&hcan, &FilterConfig) != HAL_OK)
   {
	 HAL_Error_Handler();
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

	isProcessing = 0;
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	// recv_databuf is library-wide data buffer
	uint32_t fill_level;
	fill_level = CAN_RecvMsg(CAN_RX_FIFO1, recv_databuf);

	//Test send
	//uint8_t data_send[8] = {0x31,0x32,0x33,0x34,0x31,0x32,0x33,0x34};
    //CAN_SendMsg(0,8,data_send);	//id, len, data buf

	isProcessing = 0;
}


void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan){
//#ifdef VERB
	printf("RxFifo0 is full! Hang the process..");
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 , GPIO_PIN_SET);
//#endif
	while(1);
}

void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan){
//#ifdef VERB
	printf("RxFifo1 is full! Hang the process..");
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3 , GPIO_PIN_SET);
//#endif
	while(1);
}







// initialise CAN support peripherials (GPIO & NVIC), called direcly by CAN_init
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcan->Instance==CAN)
  {
    /* Peripheral clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    /* GPIO clock and pinsel config */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

  // NVIC
  HAL_NVIC_SetPriority(30,2,0);// CEC_CAN_IRQn -> 30 , priority level ->2
  //HAL_NVIC_EnableIRQ(30); // enable CEC_CAN_IRQ


}


void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan)
{
  if(hcan->Instance==CAN)
  {
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);
  }

}


void HAL_Error_Handler(void){
	printf("HAL error occured during CAN operation!\r\n");
	while(1);
}
void CAN_Error_Handler(void){
	printf("CAN error occured during CAN operation!!\r\n");
	while(1);
}


