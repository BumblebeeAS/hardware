#include "main.h"


msg_queue uart_txbuf, uart_rxbuf;
msg_queue can_txbuf, can_rxbuf0, can_rxbuf1;
IWDG_HandleTypeDef hiwdg;
volatile uint8_t isUartIdle = 0;
uint32_t tick = 0;
uint32_t tick2 = 0;
uint32_t tick3 = 0;

// uart read frame
uint8_t incoming_data = 0;
uint8_t read_buffer[8];
uint32_t read_flag = 0;
uint32_t read_size;
uint32_t read_id;
uint32_t read_ctr;

SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim17;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
CAN_HandleTypeDef hcan;

// power control status
uint8_t power_control_status = 0xFF;

//button status
uint8_t power_button_status = 0x00;

static void MX_IWDG_Init(void);

int main(void) {

	System_Begin();
    // HW Watchdog
	// MX_IWDG_Init();
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
	MsgQueue_Init(&uart_txbuf);
	MsgQueue_Init(&uart_rxbuf);
	MsgQueue_Init(&can_rxbuf0);
	MsgQueue_Init(&can_rxbuf1);
	MsgQueue_Init(&can_txbuf);

    CAN_Begin(CAN_MODE_NORMAL);
    CAN_SetAllFilters();
    // -> can_rxbuf(ISR)
    CAN_RecvStart();
    // -> uart_rxbuf(ISR)
    // uart_txbuf(ISR) ->
    UART_RecvStart();

	while (1) {
		// static uint32_t cnt = 0;
		// cnt ++;

		if (!MsgQueue_Empty(&uart_rxbuf))
		{
			// uart_rxbuf -> can_txbuf
			SBC_Routine();
			//HAL_IWDG_Refresh(&hiwdg);

		}

		if (!MsgQueue_Empty(&can_rxbuf0) ||
		    !MsgQueue_Empty(&can_rxbuf1))
		{
			// can_rxbuf -> uart_txbuf
			CAN_Routine();
		}

		if (!MsgQueue_Empty(&uart_txbuf))
	    {
			// uart_txbuf(IRQ) ->
		    UART_TranStart();
	    }

		if (!MsgQueue_Empty(&can_txbuf))
        {
			// can_txbuf ->
	        CAN_TranStart();
        }

		if ((HAL_GetTick() - tick) > 500) {
			// -> uart_txbuf
			// -> can_txbuf
			CAN_SendHeartBeat();
		}

		if ((HAL_GetTick() - tick2) > 1000) {
			// -> uart_txbuf
			SBC_SendPowerControlStatus();
		}

		if ((HAL_GetTick() - tick3) > 50) {
			// -> uart_txbuf
			SBC_SendButtonStatus();
		}

	}

}

/****************Application Specific Functions****************/

// SBC msg -> CAN msg routine
void SBC_Routine() {
	// TODO: Add reset when error is detected

	// keep processing until empty
	// control states are in global variable,
	// so re-entering function can continue from last routine.
	while (1)
	{
		// ensure atomic access to uart_rxbuf by disableing IRQ temporarily
		HAL_NVIC_DisableIRQ(USART2_IRQn);
		uint8_t isAvailable = !MsgQueue_Empty(&uart_rxbuf);
		if (isAvailable)
			incoming_data = MsgQueue_pop(&uart_rxbuf);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
		if (!isAvailable) break;

		if (incoming_data == START_BYTE && !read_flag)
		{
			read_flag = 1;
			read_ctr = 1;
		}
		else if (incoming_data == START_BYTE && read_flag == 1)
		{
			read_flag++;
		}
		else if (read_flag == 2)
		{
			if (read_ctr == 1)
			{
				read_id = incoming_data;
				read_ctr++;
			}
			else if (read_ctr == 2)
			{
				read_size = incoming_data;
				read_ctr++;
				if(read_size > 8)
				{
					// should reset instead
					// while(1);
				}
			}
			else if (read_ctr > 2)
			{
				read_buffer[read_ctr - 3] = incoming_data;
				if (read_ctr == (2 + read_size))
				{
					// received full data
					if (read_id == 4 && read_buffer[0] == 1)
					{
						//Check for HEARTBEAT from SBC
						// ensure atomic access to uart_txbuf by disableing IRQ temporarily
						HAL_NVIC_DisableIRQ(USART2_IRQn);
						MsgQueue_push(&uart_txbuf, START_BYTE);
						MsgQueue_push(&uart_txbuf, START_BYTE);
						MsgQueue_push(&uart_txbuf, read_id);
						MsgQueue_push(&uart_txbuf, read_size);
						MsgQueue_push(&uart_txbuf, read_buffer[0]);
						// HAL_NVIC_EnableIRQ(USART2_IRQn);

					}
					if (read_id == 3 && read_size == 1) {
						// power control
						CAN_PowerCtrl(read_buffer[0]);

					}
					uint8_t ret = 1;

					ret &= MsgQueue_push(&can_txbuf, read_id);
					ret &= MsgQueue_push(&can_txbuf, read_size);
					for (uint8_t i = 0 ; i < read_size ; i++)
					{
						ret &= MsgQueue_push(&can_txbuf, read_buffer[i]);
					}
					// refresh wd timer when uart message is receivesd



					read_flag = 0;
					read_ctr = 0;

					// UART_TestIntegrity(read_id,read_buffer,read_size);

				}
				else {
					read_buffer[read_ctr - 3] = incoming_data;
					read_ctr++;
				}
			}

		}
	}
}

void UART_RecvStart()
{
	HAL_NVIC_DisableIRQ(USART2_IRQn);
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);

	isUartIdle = 0;

	// disable UART, reset status flags
	// this is bit 0 on f0 series
	huart2.Instance->CR1 &= ~(1 << 0);
	// enable RXNE interrupt
	// TXEIE and TCIE should be set only when needed
	// Because TXE will always be 1 when not transmitting
	// Hence TX interrupt will always trigger
	huart2.Instance->CR1 |= (1 << 5);
	// enable RX and UART
	// enable TX only when transmitting.
	huart2.Instance->CR1 |= (1 << 2);
	// Enable UART, bit 0 on f0 series
	huart2.Instance->CR1 |= (1 << 0);

	HAL_NVIC_EnableIRQ(USART2_IRQn);

	UART_WaitIdle(2000);

}

void UART_WaitIdle(uint32_t timeout)
{
	// wait for idle line, then begin receiving
	// otherwise random bits can be wrongly recognised as start bit
	uint32_t timoutTick = HAL_GetTick();
	uint8_t cnt = 0;
	while (!(huart2.Instance->ISR & (1 << 4)) && (HAL_GetTick() - timoutTick) < timeout)
	{
		huart2.Instance->ICR &= ~(1<<4); // clear IDLE
		cnt ++;
		if (cnt >4) break;
	}

	// TODO: Call reset handler
	isUartIdle = 1;

}


void UART_RecvPause()
{
	//HAL_NVIC_DisableIRQ(USART2_IRQn);
}

void UART_RecvResume()
{
	//HAL_NVIC_EnableIRQ(USART2_IRQn);
}


void UART_TranStart()
{
	  // enable TXE, clear msg queue in interrupt
	  huart2.Instance->CR1 |= (1 << 7);
}

void CAN_RecvStart() {

	HAL_NVIC_DisableIRQ(30);
	HAL_NVIC_SetPriority(30,3,0);

	// start notification (Interrupt)
	if (HAL_CAN_ActivateNotification(&hcan,
		CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	{
		Error_Handler();
	}


	HAL_NVIC_EnableIRQ(30);
}

void CAN_TranStart()
{
	uint32_t txmailbox;
	// configure TxHeader
	CAN_TxHeaderTypeDef txheader;
	txheader.StdId = MsgQueue_pop(&can_txbuf);
	txheader.ExtId = 0;
	txheader.DLC   = MsgQueue_pop(&can_txbuf);
	txheader.IDE   = CAN_ID_STD;
	txheader.RTR   = CAN_RTR_DATA;

	uint8_t txdata[12];
	uint8_t ret = 1;
	ret &= txheader.DLC;
	for (uint8_t i = 0 ; i < txheader.DLC ; i++)
	{
		txdata[i] = MsgQueue_pop(&can_txbuf);
		ret 	 &= txdata[i];
	}
	if (ret == 0)
	{
		//while(1);
	}
	if (HAL_CAN_AddTxMessage(&hcan, &txheader, txdata, &txmailbox ) != HAL_OK)
	{
		// while(1);
	}
}


void UART_TestIntegrity(uint8_t read_id, uint8_t* read_buffer, uint8_t read_size)
{
	// ensure integrity by checking thruster idle frames
	uint8_t ret = 1;
	if (read_size != 8) ret = 0;
	else if (read_id == 1)
	{
		uint8_t cmp[8] = {244,1,204,1,220,1,212,1};
		for(uint8_t i = 0 ; i < 8 ; i++)
		{
			if (cmp[i] != read_buffer[i]) ret = 0;
		}
	}
	else if (read_id == 0)
	{
		uint8_t cmp[8] = {189,1,244,1,244,1,244,1};
		for(uint8_t i = 0 ; i < 8 ; i++)
		{
			if (cmp[i] != read_buffer[i]) ret = 0;
		}
	} else {
		ret = 0;
	}

	if (ret == 0)
	{
		 //while(1);
	}

}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

	CAN_RxHeaderTypeDef rxheader; 		//place where the received header will be stored
	uint8_t rxdata[12];

	HAL_CAN_GetRxMessage(hcan, 0, &rxheader, rxdata);

	uint8_t id = rxheader.StdId;
	uint8_t len = rxheader.DLC;
	uint8_t ret = 1;

	ret &= MsgQueue_push(&can_rxbuf0, id);
	ret &= MsgQueue_push(&can_rxbuf0, len);
	for (uint8_t i = 0 ; i < len ; i++)
	{
		ret &= MsgQueue_push(&can_rxbuf0, rxdata[i]);
	}
	if (ret == 0)
	{
		// for now, catch buffer full
		//while(1);
	}

}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CAN_RxHeaderTypeDef rxheader; 		//place where the received header will be stored
	uint8_t rxdata[12];

	HAL_CAN_GetRxMessage(hcan, 1, &rxheader, rxdata);

	uint8_t id = rxheader.StdId;
	uint8_t len = rxheader.DLC;
	uint8_t ret = 1;

	ret &= MsgQueue_push(&can_rxbuf1, id);
	ret &= MsgQueue_push(&can_rxbuf1, len);
	for (uint8_t i = 0 ; i < len ; i++)
	{
		ret &= MsgQueue_push(&can_rxbuf1, rxdata[i]);
	}
	if (ret == 0)
	{
		// for now, catch buffer full
		// while(1);
	}

}

//CAN msg -> SBC msg routine
void CAN_Routine() {

	if (!MsgQueue_Empty(&can_rxbuf0))
	{
		uint8_t id  = MsgQueue_pop(&can_rxbuf0);
		uint8_t len = MsgQueue_pop(&can_rxbuf0);

		MsgQueue_push(&uart_txbuf, START_BYTE);
		MsgQueue_push(&uart_txbuf, START_BYTE);
		MsgQueue_push(&uart_txbuf, id);
		MsgQueue_push(&uart_txbuf, len);

		for (uint8_t i = 0 ; i < len ; i++)
		{
			MsgQueue_push(&uart_txbuf, MsgQueue_pop(&can_rxbuf0));
		}
	}

	if(!MsgQueue_Empty(&can_rxbuf1))
	{
		uint8_t id  = MsgQueue_pop(&can_rxbuf1);
		uint8_t len = MsgQueue_pop(&can_rxbuf1);

		MsgQueue_push(&uart_txbuf, START_BYTE);
		MsgQueue_push(&uart_txbuf, START_BYTE);
		MsgQueue_push(&uart_txbuf, id);
		MsgQueue_push(&uart_txbuf, len);

		for (uint8_t i = 0 ; i < len ; i++)
		{
			MsgQueue_push(&uart_txbuf, MsgQueue_pop(&can_rxbuf1));
		}
	}

}

//Toggles PCB power
void CAN_PowerCtrl(uint8_t pwr_ctr) {
	power_control_status = pwr_ctr;

	for (uint8_t i = 0; i < 8; i++) {
		if ((pwr_ctr >> i) & 0x01) {
			// If control bit is set
			// this will toggle ON
			switch (i) {
			case 0:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
				break;
			case 2:	//prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
				break;
			case 3: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
				break;
			case 4: //prb
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
				break;
			case 5: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
				break;
			case 6: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				break;
			case 7: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
				break;
			default:
				break;
			}
		} else {
			//If control bit is not set
			// This will toggle OFF
			switch (i) {
			case 0:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
				break;
			case 3:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
				break;
			case 4:
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
				break;
			case 5:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
				break;
			case 6:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				break;
			case 7:
//				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
				break;
			default:
				break;
			}
		}
	}

}

void CAN_SendHeartBeat()
{
	huart2.Instance->CR1 &= ~(1 << 7);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, 4); // hb message id
	MsgQueue_push(&uart_txbuf, 1); // len
	MsgQueue_push(&uart_txbuf, 2); // sbc_can hb id
    huart2.Instance->CR1 |= (1 << 7);

	MsgQueue_push(&can_txbuf, 4); // hb message id
	MsgQueue_push(&can_txbuf, 1); // len
	MsgQueue_push(&can_txbuf, 2); // sbc_can hb id



	tick = HAL_GetTick();
}


void SBC_SendPowerControlStatus()
{
	huart2.Instance->CR1 &= ~(1 << 7);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, 30);
	MsgQueue_push(&uart_txbuf, 1);
	MsgQueue_push(&uart_txbuf, power_control_status);
	huart2.Instance->CR1 |= (1 << 7);

	tick2 = HAL_GetTick();

}

void SBC_SendButtonStatus()
{
	power_button_status = HAL_GPIO_ReadPin (GPIOA, GPIO_PIN_8) << 1 | HAL_GPIO_ReadPin (GPIOF, GPIO_PIN_1);
	huart2.Instance->CR1 &= ~(1 << 7);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, 13); //button CAN ID
	MsgQueue_push(&uart_txbuf, 1); // len
	MsgQueue_push(&uart_txbuf, power_button_status);
	huart2.Instance->CR1 |= (1 << 7);

	tick3 = HAL_GetTick();

}


void CAN_SetAllFilters() {
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


void System_Begin() {
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	// MX_SPI1_Init();
	MX_USART2_UART_Init();
	// MX_TIM17_Init();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	// Start HSI
//
//  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//  {
//    Error_Handler();
//  }

	// Start HSE
	// Remember to change HSE_VALUE definition to 32000000
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	 RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
	  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
		if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
			Error_Handler();
		}


		  /** Initializes the RCC Oscillators according to the specified parameters
		  * in the RCC_OscInitTypeDef structure.
		  */
		  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
		  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
		  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
			 RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
			 RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
			 RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
			 RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
			  RCC_OscInitStruct.LSIState = RCC_LSI_ON;

		  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		  {
		    Error_Handler();
		  }

	// Not using PLL clock





	// Initializes the CPU, AHB and APB busses clocks
	// Use HSE for AHB, APB and sysclock
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48; //HSE
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}



}



/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_128;
  hiwdg.Init.Window = 8095;
  hiwdg.Init.Reload = 8095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}




void Error_Handler() {
	while(1);
}
