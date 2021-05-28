#include "can_lib.h"

/*#############Public############*/

void CAN_Begin(uint32_t Mode){
	hcan.Init.Mode = Mode;
	CAN_Init();

	CAN_InitFilter();
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_READY)
	{
		Error_Handler();
	}

	HAL_CAN_Start(&hcan);
	if( HAL_CAN_GetState(&hcan) != HAL_CAN_STATE_LISTENING)
	{
		Error_Handler();
	}

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
	 Error_Handler();
   }

}




void CAN_Init(void)
{
  hcan.Instance = CAN;
  hcan.Instance->MCR &= ~(1<<16);	//This line is needed for CAN to work in debug mode!!!!!!!
  hcan.Init.Prescaler = 3;
  //hcan.Init.Mode = CAN_MODE;	//This line now become a parameter for CAN_Begin function
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan) != HAL_OK)
  {
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
	   Error_Handler();
   }


}



