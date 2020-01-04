// issue log: Problem initialising GPIO PA0 PA4

#include "main.h"

#define START_BYTE 50		// (uint8_t) 254 ==> (int8_t) -2

uint8_t recv_data = 0;
uint8_t read_flag = 0;	// to validate start condition
uint8_t read_ctrl = 0;	// to know which byte is currently reading
uint8_t read_id = -1;
uint8_t read_size = 0;
uint8_t SBC_recvMsg[100] = {0};
uint8_t SBC_msgPending = 0;
uint8_t CAN_recvMsg[100] = {0};
uint32_t RxFifo = 0;


int main(void)
{
	OB_TypeDef Res;
	Res.USER |= ((1 << 1) | (1 << 2));	//disable stop mode and sleep mode
	  RCC_TypeDef RCC_S;
	  uint32_t ret = RCC_S.CSR;	//disable reset upon stop /sleep mode
	  	  	  	  	  	  	  	  //This register also report why chip reseted
	  //hackjob to solve mysterious reset issue caused by PA0 and PA4




  System_Begin();
  RetargetInit(&huart1);	//printf takes 3ms -> 333Hz max
  //Enable_TIM17();
  CAN_Begin();
  Enable_Uart_Int();

  printf("helloworld!\r\n");


  //huart2.Instance->CR3 |= (1 << 12);	//disable overrun
  while (1)
  {

//	  if (SBC_msgPending){
//		  SBC_Routine();
//	  }
	  if ((huart2.Instance->ISR >> 4) & 0x01) {	//overrun occurs
		  huart2.Instance->ICR |= 1 << 3 ;	//Clear overrun
	  }
	  if ( (huart2.Instance->ISR >> 5) & 0x01){ //RXNE is set -> Msg received
		  	  //overrun error at bit 3 will cause bus to hang
		  SBC_Routine();
	  }
	  if (RxFifo = CAN_CheckReceive() != 10 ){ //if there is msg in RxFifo
		  CAN_Routine();
	  }
	  printf("hellow!\r\n");
	 HAL_Delay(1000);

  }

}




/****************Application Specific Functions****************/


// SBC msg -> CAN msg routine
void SBC_Routine()
{
	HAL_UART_Receive(&huart2,&recv_data,1,1);	//Get next byte


	while(1){
		huart2.Instance->RDR &= 0; //clear buffer
		//printf("%lu\t%d\t%d\t%d\r\n",HAL_GetTick(),(uint8_t)recv_data,read_flag,read_ctrl);	//for debugging
		HAL_UART_Transmit(&huart2, &recv_data, 1, 1);//echo back to sender
		printf("%d\r\n",recv_data);
		if (recv_data != START_BYTE && read_flag < 2){ //two start bytes must be consecutive
			reset_Uart_Msg();
			break;	//wrong starting condition, exit
		}

		else if (recv_data == START_BYTE && read_flag < 2){	//fist Start byte
			read_flag ++;
			read_ctrl = 1;
		}

		//read_flag == 2 -> start condition validated
		else if (read_flag == 2){
			if(read_ctrl > 30){
				reset_Uart_Msg();	//boundary condition reached
				break;
			}
			else if (read_ctrl == 1){	//first byte of incoming data (id)
				read_id = recv_data;
				read_ctrl ++;
			}
			else if (read_ctrl == 2){	//second byte of incoming data (size)
				read_size = recv_data;
				read_ctrl ++;
			}
			else if (read_ctrl > 2){	//start reading data
				SBC_recvMsg[read_ctrl - 3] = recv_data; // read_ctrl == 3 -> first data byte
				if (read_ctrl == (2 + read_size)){	//reach the last data byte
					//CAN_SendMsg(read_id,SBC_recvMsg,read_size);
					reset_Uart_Msg();
					break;
				}
				else{	//continue to read data bytes
					SBC_recvMsg[read_ctrl - 3] = recv_data;
					read_ctrl ++ ;
				}
			}
		}
		recv_data = 0;
		HAL_UART_Receive(&huart2,&recv_data,1,1);	//Get next byte
		//*(huart2.pRxBuffPtr) &= 0;	//clear buffer
	}

	SBC_msgPending = 0;
	//HAL_UART_Receive_IT(&huart2,&recv_data,1);	//Listening UART port. Non blocking
}


//CAN msg -> SBC msg routine
void CAN_Routine()
{
	uint8_t Start_Byte = START_BYTE;
	uint8_t CAN_id = (uint8_t) CAN_GetId();
	uint8_t CAN_len = RxHeader.DLC;
	CAN_RecvMsg(RxFifo,CAN_recvMsg);
	HAL_UART_Transmit(&huart2, &Start_Byte, 1, 1);	//start 1
	HAL_UART_Transmit(&huart2, &Start_Byte, 1, 1);	//start 2
	HAL_UART_Transmit(&huart2, &CAN_id, 1, 1);		//ID
	HAL_UART_Transmit(&huart2, &CAN_len, 1, 1);		//Datalength
	HAL_UART_Transmit(&huart2,CAN_recvMsg,CAN_len,1);	//Data

}


//callback function for TIM17 interrupt
//called automatically by HAL_TIM_IRQHandler()
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	//TIM_Tick ++;
	//if(TIM_Tick % 1000 == 0)
		//printf("%lu\r\n",TIM_Tick/1000);
}

void Enable_Uart_Int()
{
	HAL_NVIC_SetPriority(USART2_IRQn,3,0);
	HAL_NVIC_EnableIRQ(USART2_IRQn); //enable timer
	HAL_UART_Receive_IT(&huart2,&recv_data,1);	//start receiving in interrupt mode, one byte
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	SBC_msgPending = 1;
	printf("%d\r\n",recv_data);
}


void reset_Uart_Msg(){
	read_flag = 0;	// to validate start condition
	read_ctrl = 0;	// to know which byte is currently reading
	read_id = -1;
	read_size = 0;
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
