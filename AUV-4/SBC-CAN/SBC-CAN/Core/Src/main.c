#include "main.h"

//SBC Ring buffer
uint8_t SBC_recvMsgBuf[2500] = {0};

uint16_t SBC_recvMsg_WP = 0;	//Pointer to buffer
uint16_t SBC_recvMsg_RP = 0;	//Pointer to buffer
uint8_t SBC_msgSize = 255;	//-1 , no msg received
uint8_t SBC_msgRdy = 0;



uint8_t SBC_msgPending = 0;		//start byte

uint32_t RxFifo = 10;

//Timer tick
uint32_t TIM_Tick = 0;

uint32_t tick_50hz = 0;
uint32_t tick_10hz = 0;
uint32_t tick_20hz = 0;
uint32_t tick_70hz = 0;

uint32_t cnt = 0;

int main(void)
{

  System_Begin();			//system and peripheral begin
  RetargetInit(&huart1);	//Printf through FTDI
  //Enable_TIM17();			//timer17 interrupt


  CAN_Begin(CAN_MODE_NORMAL);		//CAN begin
  CAN_SetAllFilters();


  Enable_Uart_Int();		//start listening to SBC

  printf("Hi! I am SBC-CAN AUV4.0!\r\n");

  while (1)
  {

	  if (SBC_msgPending >= 2){//Msg received, new msg size is updated. If size>8 -> invalid

		  SBC_Routine();
		  cnt ++;
		 // SBC_msgRdy = 0;
	  }

	  RxFifo = CAN_CheckReceive();
	  if (RxFifo != 10 ){ //if there is msg in RxFifos
		  CAN_Routine();
	  }


	  if (SBC_recvMsg_WP > 2483 && !SBC_msgPending)	//Reaching the end of buffer and no message is incoming
	  {
		  while( HAL_UART_AbortReceive_IT(&huart2)!= HAL_OK); //Kill reception
		  HAL_UART_Receive_IT(&huart2, (uint8_t*)SBC_recvMsgBuf , 2500);	//start receiving again
		  SBC_recvMsg_WP = 0;

	  }


	  //test can inspector
//	  while (1)
//	  {
//		  uint8_t msg[8] = {5};
//		  if ((HAL_GetTick()-tick_50hz) > 20){
//			  CAN_SendMsg(0,msg,8);
//			  tick_50hz = HAL_GetTick();
//		  }
//		  if ((HAL_GetTick()-tick_10hz) > 100){
//			  CAN_SendMsg(1,msg,8);
//			  tick_10hz = HAL_GetTick();
//		  }
//		  if ((HAL_GetTick()-tick_20hz) > 50){
//			  CAN_SendMsg(2,msg,8);
//			  tick_20hz = HAL_GetTick();
//		  }
//		  if ((HAL_GetTick()-tick_70hz) > 14){
//			  CAN_SendMsg(3,msg,8);
//			  tick_70hz = HAL_GetTick();
//		  }
//
//	  }

	  // error message

	  // uptime/heartbeat


  }

}



/****************Application Specific Functions****************/

// SBC msg -> CAN msg routine
void SBC_Routine()
{
	SBC_msgPending = 0;			//clear control status
	SBC_msgSize = 255;
	HAL_Delay(3);
	SBC_ReceptionHandler(SBC_recvMsgBuf + SBC_recvMsg_RP);
}


// Convert SBC msg to CAN msg and send
void SBC_ReceptionHandler(uint8_t *recvMsgBuf){

	uint8_t id = recvMsgBuf[0], size = recvMsgBuf[1], i = 0;


	uint8_t CAN_sendMsgBuf[8] = {0};

	// For debugging
//	printf("\r\n===========SBC_ReceptionHandler===========\r\n");
//	printf("id:\t%d\r\nsize:\t%d\r\n",id,size);
//	printf("data: ");
	//transfer SBC msg buf into CAN msg buf
	for ( i = 0 ; i < size ; i++){
		//printf("%d\t",recvMsgBuf[i+2]);
		CAN_sendMsgBuf[i] = recvMsgBuf[i+2];
	}

	if (id != 5){
		printf("hello?!");
	}
	//send msg
	CAN_SendMsg(id,CAN_sendMsgBuf,size);
}

void Enable_Uart_Int()
{
	HAL_NVIC_SetPriority(USART2_IRQn,3,0);
	HAL_NVIC_EnableIRQ(USART2_IRQn); //enable timer

	SBC_recvMsg_WP = 0;
	HAL_UART_Receive_IT(&huart2, (uint8_t*)(SBC_recvMsgBuf) , 2500);	//start receiving in interrupt mode, 32 byte buffer

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	SBC_recvMsg_WP = 0;
	HAL_UART_Receive_IT(&huart2, SBC_recvMsgBuf , 2500);	//start receiving in same buffer
}



//CAN msg -> SBC msg routine
void CAN_Routine()
{
	uint8_t CAN_recvMsg[32] = {0};
	CAN_RecvMsg(RxFifo,CAN_recvMsg);	//receive a can message

	if (CAN_GetId() == CAN_PCB_STAT){
		CAN_PowerCtrl(CAN_recvMsg);
	}
	else{
		CAN_ReceptionHandler(CAN_recvMsg);
	}

	// for debugging
	/*
	printf("\r\n===========CAN_ReceptionHandler===========\r\n");
	printf("%d\t%d\t%d\t%d\t\r\n",Start_Byte,Start_Byte,CAN_id,CAN_len);
	uint8_t i = 0;
	for ( i = 0 ; i < CAN_len ; i++){
		printf("%d\t",CAN_recvMsg[i]);
	}
	printf("\r\n");
	*/

}


// Convert CAN msg to SBC msg and send
void CAN_ReceptionHandler(uint8_t* recvMsgBuf)
{

	uint8_t SBC_sendMsgBuf[16] = {0};
	SBC_sendMsgBuf[0] = START_BYTE;
	SBC_sendMsgBuf[1] = START_BYTE;
	SBC_sendMsgBuf[2] = CAN_GetId();
	SBC_sendMsgBuf[3] = RxHeader.DLC;
	uint8_t i = 0;
	for ( i = 0 ; i < RxHeader.DLC ; i++){
		SBC_sendMsgBuf[4+i] = recvMsgBuf[i];
	}
	HAL_UART_Transmit(&huart2, SBC_sendMsgBuf, (4 + RxHeader.DLC), 1);	//start 1
}


//Toggles PCB power
void CAN_PowerCtrl(uint8_t* recvMsgBuf)
{

	uint8_t i = 0;
	uint8_t PwrCtrl = recvMsgBuf[0];
	for ( i = 0 ; i < 8 ; i ++){
		PwrCtrl = PwrCtrl >> i; //Check each bit in the msg;
		if ((PwrCtrl & 0x01)){	//If control bit is set
			switch (i) {
				case 0 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
					break;
				case 1 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
					break;
				case 2 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
					break;
				case 3 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
					break;
				case 4 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
					break;
				case 5 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
					break;
				case 6 :
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
					break;
				case 7 :
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
					break;
				default:
					break;
			}
		}
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
