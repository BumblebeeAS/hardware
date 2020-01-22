#include "main.h"



//SBC receive buffer
uint8_t SBC_recvMsgBuf[256] = {0};
uint8_t SBC_WP = 0;			// this is updated whenever a byte comes in, could be rubbish data
uint8_t SBC_RP = 0;			// this is updated when a valid message is detected and point to the first msg byte (Id)
uint8_t SBC_Last_RP = 0;	// this is updated when a message is executed. Compare this pointer to SBC_RP to check if new message has come in
//sbc control variables
uint8_t SBC_msgPending = 0;		//start byte

//CAN receive buffer
uint8_t CAN_recvMsgBuf[256] = {0};
uint8_t CAN_WP = 0;	//This always points to to the end of the last message. So simply need to compare RP with WP/
uint8_t CAN_RP = 0;	//LAST_RP is not needed in this case as a CAN message will always be valid. (Filtered by CAN controller)
//CAN control variables
uint32_t RxFifo = 10;



//Timer tick
uint32_t TIM_Tick = 0;

uint32_t cnt = 0;
uint32_t tick = 0;
uint8_t pwr = 0;

int main(void)
{

  System_Begin();			//system and peripheral begin
  RetargetInit(&huart1);	//Printf through FTDI
  //Enable_TIM17();			//timer17 interrupt


  CAN_Begin(CAN_MODE_NORMAL);		//CAN begin
  CAN_SetAllFilters();
	CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len



  Enable_Uart_Int();		//start listening to SBC

  printf("Hi! I am SBC-CAN AUV4.0!\r\n");

  while (1)
  {

	  if (SBC_Last_RP != SBC_RP){
		  SBC_Routine();
	  }

	  if (CAN_RP != CAN_WP ){ //if there is msg in RxFifos
		  CAN_Routine();
	  }

	  if ((HAL_GetTick() - tick) > 100){
		  uint8_t heartbeatId[1] = {2}; //sbc_can id
		  tick = HAL_GetTick();
		  CAN_SendMsg(6,heartbeatId,1); //id,msg,len
	  }

	  // error message

	  // uptime/heartbeat


  }

}



/****************Application Specific Functions****************/

// SBC msg -> CAN msg routine
void SBC_Routine()
{
	uint8_t Id = SBC_recvMsgBuf[SBC_RP];
	uint8_t Size = SBC_recvMsgBuf[SBC_RP+1];
	if (SBC_RP > 243 && SBC_WP < 12 && ((255-SBC_RP)+SBC_WP) < (Size) ){	//boundary condition
		return;
	}
	else if ( ((SBC_WP - SBC_RP) < Size+1 ) && !(SBC_RP > 243 && SBC_WP < 12)) {	//Msg having complete yet
		return;
	}
	else{//msg complete
		if (Id == 3){ //power control
			CAN_PowerCtrl(SBC_recvMsgBuf+SBC_RP+2);
		}
		else {	//publish SBC msg
			CAN_SendMsg(Id,SBC_recvMsgBuf+SBC_RP+2,Size);
			SBC_Last_RP = SBC_RP;
			  cnt ++;
		}
	}
}


void Enable_Uart_Int()
{
	HAL_NVIC_SetPriority(USART2_IRQn,3,0);
	HAL_NVIC_EnableIRQ(USART2_IRQn); //enable timer
	SBC_msgPending = 0;
	HAL_UART_Receive_IT(&huart2, SBC_recvMsgBuf , 1);	//start receiving in interrupt mode, 32 byte buffer
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (SBC_msgPending == 4){
		SBC_RP = SBC_WP - 2;	//move RP to ID
		SBC_msgPending = 0;
	}
	else if (SBC_msgPending >= 2){	//wait 2 more bytes for size data
		SBC_msgPending ++;
	}
	else if ( *(SBC_recvMsgBuf + SBC_WP) == START_BYTE){
		SBC_msgPending ++;	//count no of start bytes
	}

	SBC_WP++;	//Update write pointer
	HAL_UART_Receive_IT(&huart2, SBC_recvMsgBuf + SBC_WP , 1);	//start receiving in interrupt mode, 32 byte buffer
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
	CAN_recvMsgBuf[CAN_WP] = (uint8_t) (RxHeader.StdId);	//upper
	CAN_recvMsgBuf[CAN_WP+1] = (uint8_t) (RxHeader.DLC);	//len
	CAN_WP += RxHeader.DLC + 2;	//incrememnt WP

}


void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
	CAN_recvMsgBuf[CAN_WP] = (uint8_t) (RxHeader.StdId);	//upper
	CAN_recvMsgBuf[CAN_WP+1] = (uint8_t) (RxHeader.DLC);	//len
	CAN_WP += RxHeader.DLC + 2;	//incrememnt WP
}


//CAN msg -> SBC msg routine
void CAN_Routine()
{
	uint8_t Id = CAN_recvMsgBuf[CAN_RP];
	uint8_t Size = CAN_recvMsgBuf[CAN_RP + 1];
	if (Id == 3){
		CAN_PowerCtrl(CAN_recvMsgBuf+CAN_RP+2);
	}
	else{	//send SBC Msg
		uint8_t SBC_sendMsgBuf[16] = {0};
		SBC_sendMsgBuf[0] = START_BYTE;
		SBC_sendMsgBuf[1] = START_BYTE;
		SBC_sendMsgBuf[2] = Id;
		SBC_sendMsgBuf[3] = Size;
		uint8_t i = 0;
		for ( i = 0 ; i < Size ; i++){	//fill msg
			SBC_sendMsgBuf[i+4] = CAN_recvMsgBuf[CAN_RP+2+i];
		}
		HAL_UART_Transmit(&huart2, SBC_sendMsgBuf, (4 + Size), 1);
	}
	CAN_RP += (2 + Size);
}


//Toggles PCB power
void CAN_PowerCtrl(uint8_t* recvMsgBuf)
{

	uint8_t i = 0;
	uint8_t PwrCtrl = recvMsgBuf[0];
	for ( i = 0 ; i < 8 ; i ++){
		if ((PwrCtrl >> i) & 0x01){	//If control bit is set
			switch (i) {
				case 0 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
					break;
				case 1 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
					break;
				case 2 ://prob
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
					break;
				case 3 : //prob
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
					break;
				case 4 : //prb
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
					break;
				case 5 ://prob
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
					break;
				case 6 ://prob
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
					break;
				case 7 ://prob
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
					break;
				default:
					break;
			}
		}
		else {	//If control bit is not set
			switch (i) {
				case 0 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
					break;
				case 1 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
					break;
				case 2 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
					break;
				case 3 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
					break;
				case 4 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
					break;
				case 5 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
					break;
				case 6 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
					break;
				case 7 :
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
					break;
				default:
					break;
				}
		}
		HAL_Delay(10);
	}

}





void CAN_SetAllFilters()
{
	// ID 0 ~ 15 goes to FIFO0
	// Filter configuration:
	// ID: 		0b00001111
	// Mask:	0b11110000
	// Effect:  Accept all IDs below 16
	CAN_SetFilter(0x0F, 0xF0, 0x0F, 0xF0, 1, CAN_RX_FIFO0);
	// ID 16 ~  goes to FIFO1
	// Filter configuration:
	// ID: 		0x00
	// Mask:	0x00
	// Effect:  Accept all. But since 0~15 will go into filter bank with higher priority,
	//          only 16~ will go here
	CAN_SetFilter(0x00, 0x00, 0x00, 0x00, 2, CAN_RX_FIFO1);


}






//enable NVIC, reset TIM_Tick and starts TIM17 (base mode)
void Enable_TIM17(void)
{
	HAL_NVIC_SetPriority(TIM17_IRQn,3,0);
	HAL_NVIC_EnableIRQ(TIM17_IRQn); //enable timer
	TIM_Tick = 0;	//reset TIM_Tick
	HAL_TIM_Base_Start_IT(&htim17);
}



void System_Begin()
{

	//Peripheral inits are in the peripheral_Init.c
	Error_Status = 0;	//reset error status
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_TIM17_Init();

	  //hackjob to solve mysterious reset issue caused by PA0 and PA4(?)
	OB_TypeDef Res;
	Res.USER |= ((1 << 1) | (1 << 2));	//disable stop mode and sleep mode
	//RCC_TypeDef RCC_S;
	// RCC_S.CSR;	//disable reset upon stop /sleep mode
	  	  	  	  	 //This registerreport why chip reseted


}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  // Start HSI
//
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Init_Error_Handler();
//  }


  // Start HSE
  // Remember to change HSE_VALUE definition to 32000000
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
   RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  // Not using PLL clock
/*
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
*/
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
	  Error_Status |= 1 << SYSCLOCK_INIT_ERROR; //updata error
	  Error_Handler();
  }


  // Initializes the CPU, AHB and APB busses clocks
  // Use HSE for AHB, APB and sysclock
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
	  Error_Status |= 1 << SYSCLOCK_INIT_ERROR;	//update error
	  Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
	  Error_Status |= 1 << SYSCLOCK_INIT_ERROR;	//update error
	  Error_Handler();
  }
}


//callback function for TIM17 interrupt
//called automatically by HAL_TIM_IRQHandler()
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	//TIM_Tick ++;
	//if(TIM_Tick % 1000 == 0)
		//printf("%lu\r\n",TIM_Tick/1000);
}


void Error_Handler()
{
	uint8_t i = 0;
	for (i = 0 ; i < 32 ; i++){
		if ( (Error_Status >> i) & 0x01 ){
			PrintError(i);
		}
	}


	while(1);

}



#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
