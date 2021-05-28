#include "main.h"

msg_queue uart_txbuf, uart_rxbuf;
msg_queue can_txbuf, can_rxbuf0, can_rxbuf1;
uint8_t isIdle = 0;
uint32_t tick = 0;


// uart read frame
uint8_t incoming_data = 0;
uint8_t read_buffer[8];
uint32_t read_flag = 0;
uint32_t read_size;
uint32_t read_id;
uint32_t read_ctr;

int main(void) {

	System_Begin();

    CAN_Begin(CAN_MODE_NORMAL);		//CAN begin
    CAN_SetAllFilters();
    CAN_RecvStart();

    UART_RecvStart();
    // check uart_rxbuf
//    while(1);

	while (1) {
		static uint32_t cnt = 0;
		cnt ++;

		if (!MsgQueue_Empty(&uart_rxbuf))
		{
			SBC_Routine();
		}

		if (!MsgQueue_Empty(&can_rxbuf0) ||
		    !MsgQueue_Empty(&can_rxbuf1))
		{
			CAN_Routine();
		}

		if (!MsgQueue_Empty(&uart_txbuf))
	    {
		    UART_TranStart();
	    }

		if (!MsgQueue_Empty(&can_txbuf))
        {
	        CAN_TranStart();
        }

		if ((HAL_GetTick() - tick) > 500) {
			CAN_SendHeartBeat();
		}
	}

}

/****************Application Specific Functions****************/

// SBC msg -> CAN msg routine
void SBC_Routine() {

	// keep processing until empty
	// control states are in global variable,
	// so re-entering function can continue from last routine.
	while (!MsgQueue_Empty(&uart_rxbuf))
	{
		incoming_data = MsgQueue_pop(&uart_rxbuf);

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
					//Check for HEARTBEAT from SBC
					if (read_id == 4 && read_buffer[0] == 1)
					{
						MsgQueue_push(&uart_txbuf, START_BYTE);
						MsgQueue_push(&uart_txbuf, START_BYTE);
						MsgQueue_push(&uart_txbuf, read_id);
						MsgQueue_push(&uart_txbuf, read_size);
						MsgQueue_push(&uart_txbuf, read_buffer[0]);
					}
					if (read_id == 3) { //power control
						// check power control
					}
//					CAN.sendMsgBuf(read_id, 0, read_size, read_buffer);
					uint8_t ret = 1;

					ret &= MsgQueue_push(&can_txbuf, read_id);
					ret &= MsgQueue_push(&can_txbuf, read_size);
					for (uint8_t i = 0 ; i < read_size ; i++)
					{
						ret &= MsgQueue_push(&can_txbuf, read_buffer[i]);
					}

					read_flag = 0;
					read_ctr = 0;

					// ensure integrity
					static uint32_t cnt = 0;
					if (read_id == 1)
					{
						if (read_size != 8) ret = 0;
						uint8_t cmp[8] = {244,1,204,1,220,1,212,1};
						for(uint8_t i = 0 ; i < 8 ; i++)
						{
							if (cmp[i] != read_buffer[i]) ret = 0;
						}
						cnt ++;
					}
					else if (read_id == 0)
					{
						if (read_size != 8) ret = 0;
						uint8_t cmp[8] = {189,1,244,1,244,1,244,1};
						for(uint8_t i = 0 ; i < 8 ; i++)
						{
							if (cmp[i] != read_buffer[i]) ret = 0;
						}
						cnt ++;
					}

					if (ret == 0)
					{
						// for now, catch buffer full
						 //while(1);
					}



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
	HAL_NVIC_DisableIRQ(USART2_IRQn); //enable timer
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);

	MsgQueue_Init(&uart_txbuf);
	MsgQueue_Init(&uart_rxbuf);
	isIdle = 0;

	// disable UART, reset status flags
	// this is bit 0 on f0 seris
	huart2.Instance->CR1 &= ~(1 << 0);
	// enable RXNE interrupt
	// TXEIE and TCIE should be set only when needed
	// Because TXE will always be 1 when not transmitting
	// Hence TX interrupt will always trigge
	huart2.Instance->CR1 |= (1 << 5);
	// enable RX and UART
	// enable TX only when transmitting.
	huart2.Instance->CR1 |= (1 << 2);
	// Enable UART, enable bit is bit 0 on f0 series
	huart2.Instance->CR1 |= (1 << 0);

	HAL_NVIC_EnableIRQ(USART2_IRQn);

	// wait for idle line, then begin receiving
	// otherwise random bits can be wrongly recognised as start bit
    uint32_t timoutTick = HAL_GetTick();
    uint8_t cnt = 0;
    while (!(huart2.Instance->ISR & (1 << 4)) && (HAL_GetTick() - timoutTick) < 10000)
    {
        huart2.Instance->ICR &= ~(1<<4); // clear IDLE
    	cnt ++;
        if (cnt >4) break;
    }

    isIdle = 1;
}

void UART_TranStart()
{
	  // enable TXE, clear msg queue in interrupt
	  huart2.Instance->CR1 |= (1 << 7);
}

void CAN_RecvStart() {
	MsgQueue_Init(&can_rxbuf0);
	MsgQueue_Init(&can_rxbuf1);

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
		// for now, catch error
		//while(1);
	}
	if (HAL_CAN_AddTxMessage(&hcan, &txheader, txdata, &txmailbox ) != HAL_OK)
	{
		// for now, catch error
		// while(1);
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
void CAN_PowerCtrl(uint8_t *recvMsgBuf) {

	uint8_t i = 0;
	uint8_t PwrCtrl = recvMsgBuf[0];
	for (i = 0; i < 8; i++) {
		if ((PwrCtrl >> i) & 0x01) {	//If control bit is set
			switch (i) {
			case 0:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
				break;
			case 2:	//prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
				break;
			case 3: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
				break;
			case 4: //prb
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
				break;
			case 5: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
				break;
			case 6: //prob
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				break;
			case 7: //prob
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
				break;
			default:
				break;
			}
		} else {	//If control bit is not set
			switch (i) {
			case 0:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				break;
			case 1:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
				break;
			case 3:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
				break;
			case 4:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
				break;
			case 5:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
				break;
			case 6:
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
				break;
			case 7:
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
				break;
			default:
				break;
			}
		}
	}

}

void CAN_SendHeartBeat() {

	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, START_BYTE);
	MsgQueue_push(&uart_txbuf, 4); // hb message id
	MsgQueue_push(&uart_txbuf, 1); // len
	MsgQueue_push(&uart_txbuf, 2); // sbc_can hb id

	MsgQueue_push(&can_txbuf, 4); // hb message id
	MsgQueue_push(&can_txbuf, 1); // len
	MsgQueue_push(&can_txbuf, 2); // sbc_can hb id

	tick = HAL_GetTick();
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

	// Not using PLL clock
	/*
	 RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	 RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	 RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	 */
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

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

void Error_Handler() {
	while(1);
}
