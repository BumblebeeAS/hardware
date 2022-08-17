#include "main.h"

//CAN control variables
uint32_t RxFifo = 10;


//Timer tick
uint32_t TIM_Tick = 0;

//take in 1500 msgs, 1000 * 3 ~= 3100
// Pos 	0			1			2
// 		Time(low)	Time(high) 	Id
uint8_t TrafficLog[3100] = {0};
uint16_t log_cnt = 0;

// pos	0			1				2				3										4					5
//		Id_cnt			Last tick(low)	Last tick(High)	Duration(low)(used for freq estimate) 	Duration(High)		Diagnose
//uint8_t TrafficReport[250] = {0};	//num of id ~= 36 => 36*6
uint16_t TrafficReport[150] = {0};

uint32_t global_tick = 0;
int main(void)
{

  System_Begin();			//system and peripheral begin
  RetargetInit(&huart2);	//Printf through RS232 On Can Protoboard
  //Enable_TIM17();			//timer17 interrupt
  CAN_Begin(CAN_MODE_NORMAL);		//CAN begin
  CAN_SetAllFilters();

  printf("Hi! I am CAN Inspector AUV4.0!\r\n");

  uint8_t msgbuf = 1;
  uint32_t loopcnt = 0;
  while (1)
  {
	  uint32_t msgcnt = 0;


	  global_tick = HAL_GetTick();
	  while ( msgcnt < 10 )
	  {
		  RxFifo = CAN_CheckReceive();
		  if (RxFifo != 10 ){ //if there is msg in RxFifos
			  CAN_Log(RxFifo);
			  msgcnt ++;
		  }
	  }
	  global_tick = HAL_GetTick() - global_tick;

	  CAN_PrintTrafficLog();
	  //CAN_GenerateReport();
	 // CAN_GenerateReport2();
	  loopcnt ++;
	  printf("time passed: %lums\r\nmsg cnt: %lu\r\n",global_tick,loopcnt);


	  //reset
	  uint16_t i = 0;
	  for (i = 0 ; i < 3100 ; i ++){
		  TrafficLog[i] = 0;
	  }
	  for (i = 0 ; i < 150 ; i ++){
		  TrafficReport[i] = 0;
	  }
	  log_cnt = 0;
	  //delay 10s
	 HAL_Delay(500);

  }

}



/****************Application Specific Functions****************/



void CAN_Log(uint32_t RxFifo)
{
	uint8_t msgdump[8];
	uint32_t tick = 0;
	tick = HAL_GetTick();
	CAN_RecvMsg(RxFifo, msgdump);
	//printf("ID: %lu\r\n", CAN_GetId());

	//update log
	TrafficLog[log_cnt++] = tick;
	TrafficLog[log_cnt++] = tick >>8 ;
	TrafficLog[log_cnt++] = CAN_GetId();

}


void CAN_PrintTrafficLog()
{
	uint16_t i = 0;
	printf("\r\n===Printing CAN Traffic Log===\r\n");
	for (i = 0 ; i < log_cnt ; i +=3 )
	{
		uint32_t tick = (TrafficLog[i]) | (TrafficLog[i+1] << 8); //reassemble tick
		printf("%lu\t%d\r\n",tick,TrafficLog[i+2]); //print tick + id
	}
}


void CAN_GenerateReport2()
{
	uint16_t i = 0;
	printf("\r\n===Printing CAN Traffic Report===\r\n");
	for ( i = 0 ; i < log_cnt ; i += 3)
	{
		//Log arr offset == 3
		//report arr offset == 6
		TrafficReport[TrafficLog[i+2]*3] ++; 							//increment counter at id position (0)

 	}

	// process report
	// processed report format
	// pos	0			1				2				3				4			5
	//		ID_cnt		ID				unused			unused 			unused		unused
	for ( i = 0 ; i < 120 ; i += 3)
	{
		TrafficReport[i+1] = i / 3 ;	//store actual id instead
	}

	//  print
	for ( i = 0 ; i < 120 ; i += 3)
	{
		uint32_t freq = TrafficReport[i]*1000/global_tick;
		//printf("id: %d",TrafficReport[i+1]);
		//printf("idcnt: %d",TrafficReport[i]);
		//printf("freq: %d",freq);
		printf("id: %lu\tIDcnt: %lu\tFreq: %lu\r\n",TrafficReport[i+1],TrafficReport[i],(uint16_t)freq);

	}


}

void CAN_GenerateReport()
{
	uint16_t i = 0;
	printf("\r\n===Printing CAN Traffic Report===\r\n");

	//tally data from Log to report

	// tallied report format
	// pos	0			1				2				3										4					5
	//		Id_cnt			Last tick(low)	Last tick(High)	Duration(low)(used for freq estimate) 	Duration(High)		Diagnose
	for ( i = 0 ; i < log_cnt ; i += 3)
	{
		//Log arr offset == 3
		//report arr offset == 6

		TrafficReport[TrafficLog[i+2]*6] ++; 							//increment counter at id position (0)

		uint32_t t_tick = 0; //temporary tick
		uint32_t t_duration = 0; //temporary duration
		t_tick = (TrafficLog[i]) | (TrafficLog[i+1] << 8); //get curr timestamp

		t_tick = t_tick - (TrafficReport[TrafficLog[i+2]*6 + 1] | TrafficReport[TrafficLog[i+2]*6 + 2] << 8); //curr timestap - last time stamp
		t_duration = TrafficReport[TrafficLog[i+2]*6 + 3] | (TrafficReport[TrafficLog[i+2]*6 + 4] << 8);	//get last duration
		t_duration += t_tick; // update to new duration

		TrafficReport[TrafficLog[i+2]*6 + 1] = (TrafficLog[i]) ;		// update Last tick(low) position(1)
		TrafficReport[TrafficLog[i+2]*6 + 2] = (TrafficLog[i+1]) ;		// update Last tick(high) position(2)
		TrafficReport[TrafficLog[i+2]*6 + 3] = t_duration ;				// update duration(low)		position(3)
		TrafficReport[TrafficLog[i+2]*6 + 4] = t_duration >> 8 ;		// update duration(high)	position(4)
 	}

	// process report
	// processed report format
	// pos	0			1				2				3				4			5
	//		Id			freq			diagnose			unused 			unused		unused
	for ( i = 0 ; i < 250 ; i += 6)
	{
		uint16_t duration = TrafficReport[i+3] | TrafficReport[1+4] << 8; //get duration
		TrafficReport[i+1] = TrafficReport[i] * 1000 / duration; //calculate frequency
		TrafficReport[i] = i / 6;	//discard IDcnt, store actual id instead
	}

	// diagnose and print
	for ( i = 0 ; i < 250 ; i += 6)
	{
		TrafficReport[i+2] = CAN_FreqDiagnostic(TrafficReport[i],TrafficReport[i+1]);
		printf("id: %d\tfreq: %d          diagnose: %d\r\n",TrafficReport[i],TrafficReport[i+1],TrafficReport[i+2]);
	}

}

uint8_t CAN_FreqDiagnostic(uint8_t id, uint8_t freq)
{
	uint8_t ans = 0;
	switch (id)
	{
	case 0:
		if (freq < 70 && freq >> 30)
			ans = 1;
		break;
	case 1:
		if (freq < 70 && freq >> 30)
			ans = 1;
		break;
	case 2:
		if (freq < 70 && freq >> 30)
			ans = 1;
		break;
	case 3:
		if (freq != 0)
			ans = 1;
		break;
	case 4:
		if (freq < 15 && freq >> 5)
			ans = 1;
		break;
	case 5:
		if (freq < 70 && freq >> 30)
			ans = 1;
		break;
	case 6:
		if (freq != 0)
			ans = 1;
		break;
	case 7:
		if (freq != 0)
			ans = 1;
		break;
	case 8:
		if (freq != 0)
			ans = 1;
		break;
	case 9:
		if (freq != 0)
			ans = 1;
		break;
	case 10:
		if (freq != 0)
			ans = 1;
		break;
	case 11:
		if  (freq < 30 && freq >> 10)
			ans = 1;
		break;
	case 12:
		if (freq != 0)
			ans = 1;
		break;
	case 13:
		if (freq < 15 && freq >> 5)
			ans = 1;
		break;
	case 14:
		if (freq < 15 && freq >> 5)
			ans = 1;
		break;
	case 15:
		if (freq < 15 && freq >> 5)
			ans = 1;
		break;
	case 16:
		if (freq != 0)
			ans = 1;
		break;
	case 17:
		if  (freq < 30 && freq >> 10)
			ans = 1;
		break;
	case 18:
		if  (freq < 30 && freq >> 10)
			ans = 1;
		break;
	case 19:
		if (freq != 0)
			ans = 1;
		break;

	case 20:
		if (freq != 0)
			ans = 1;
		break;
	case 21:
		if (freq != 0)
			ans = 1;
		break;
	case 22:
		if (freq != 0)
			ans = 1;
		break;
	case 23:
		if (freq < 9 && freq >> 2)
			ans = 1;
		break;
	case 24:
		if (freq != 0)
			ans = 1;
		break;
	case 25:
		if (freq < 9 && freq >> 2)
			ans = 1;
		break;
	case 26:
		if (freq != 0)
			ans = 1;
		break;
	case 32:
		if (freq != 0)
			ans = 1;
		break;
	case 33:
		if (freq != 0)
			ans = 1;
		break;
	case 34:
		if (freq != 0)
			ans = 1;
		break;
	case 35:
		if (freq != 0)
			ans = 1;
		break;
	case 36:
		if (freq != 0)
			ans = 1;
		break;
	default:
		break;
	}

	return ans;

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
	//MX_USART1_UART_Init();
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

//
//  // Start HSE
//  // Remember to change HSE_VALUE definition to 32000000
//   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//   RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
//   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//
//  // Not using PLL clock
///*
//  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
//  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
//*/
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
//  {
//	  Error_Status |= 1 << SYSCLOCK_INIT_ERROR; //updata error
//	  Error_Handler();
//  }


  // Initializes the CPU, AHB and APB busses clocks
  // Use HSE for AHB, APB and sysclock
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
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
