/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_lib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SIGNATURE 0x3F
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */
void Set_PWM(TIM_HandleTypeDef *htim, uint32_t Channel, uint16_t val);
void checkCANmsg(void);

void CAN_RecvStart();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// CAN Buffer
uint8_t bufFifo0[8] = {0};
uint8_t bufFifo1[8] = {0};

// CAN Tick
uint32_t prevHBTick = 0;
uint32_t prevPWMTick = 0;
uint32_t prevConfigTick = 0;

// Thruster RPM
uint16_t thruster1RPM = 0;
uint16_t thruster2RPM = 0;
uint16_t thruster3RPM = 0;
uint16_t thruster4RPM = 0;
uint16_t thruster5RPM = 0;
uint16_t thruster6RPM = 0;
uint16_t thruster7RPM = 0;
uint16_t thruster8RPM = 0;

// CAN Update Interval
uint16_t rpmInterval = 1000;
uint16_t pwmInterval = 1000;
uint16_t statusInterval = 1000;
uint16_t actInterval = 1000;

//thruster latch
uint32_t prevframe1tick = 0;
uint32_t prevframe2tick = 0;
uint32_t frame1flag = 0;
uint32_t frame2flag = 0;
uint32_t frame1interval = 500;
uint32_t frame2interval = 500;



// CAN
uint8_t* CAN_recvMsgBuf;
uint8_t CAN_WP = 0;
uint8_t CAN_RP = 0;
uint8_t CAN_full = 0;



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  CAN_Begin(CAN_MODE_NORMAL);
  CAN_SetFilter(0x0F, 0xF0, 0x0F, 0xF0, 1, CAN_RX_FIFO0);
  CAN_SetFilter(0x00, 0x00, 0x00, 0x00, 2, CAN_RX_FIFO1);
  CAN_RecvStart();


  HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
  /* USER CODE END 2 */

  do{
  		CAN_recvMsgBuf = (uint8_t*) malloc(256*sizeof(uint8_t));
  	} while (CAN_recvMsgBuf == NULL);
  CAN_RecvStart();


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  if (CAN_WP != CAN_RP ){
		  UpdatePWM();
	  }


    /* USER CODE BEGIN 3 */
//	  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 2499);

	  if(frame1flag == 1)
	  {
		  prevframe1tick = HAL_GetTick();
		  frame1flag = 0;
	  }
	  if((HAL_GetTick() - prevframe1tick) >= frame1interval)
	  {
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 2999);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 2999);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 2999);
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 2999);
	  }


	  if(frame2flag == 1)
	  {
		  prevframe2tick = HAL_GetTick();
		  frame2flag = 0;
	  }
	  if((HAL_GetTick() - prevframe2tick) >= frame2interval)
	  {
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 2999);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 2999);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 2999);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 2999);
	  }

	  // CAN Heartbeat
	  if((HAL_GetTick() - prevHBTick) >= 500)
	  {
		  uint8_t buf[1] = {0x04};
		  CAN_SendMsg(4, buf, 1);
		  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
		  prevHBTick = HAL_GetTick();
	  }

	  // CAN PWM
	  if((HAL_GetTick() - prevPWMTick) >= 1000) //change back to 1000
	  {
		  uint8_t buf[8] = {0};

		  buf[0] = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1) & 0x0FF;
		  buf[1] = (__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1) >> 8) & 0x0FF;
		  buf[2] = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2) & 0x0FF;
		  buf[3] = (__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2) >> 8) & 0x0FF;
		  buf[4] = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_3)  & 0x0FF;
		  buf[5] = (__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_3) >> 8) & 0x0FF;
		  buf[6] = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4) & 0x0FF;
		  buf[7] = (__HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_4) >> 8) & 0x0FF;
		  CAN_SendMsg(19, buf, 8);

		  buf[0] = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1) & 0x0FF;
		  buf[1] = (__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_1) >> 8) & 0x0FF;
		  buf[2] = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2) & 0x0FF;
		  buf[3] = (__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2) >> 8) & 0x0FF;
		  buf[4] = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_3)  & 0x0FF;
	  	  buf[5] = (__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_3) >> 8) & 0x0FF;
		  buf[6] = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_4) & 0x0FF;
		  buf[7] = (__HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_4) >> 8) & 0x0FF;
		  CAN_SendMsg(20, buf, 8);

		  prevPWMTick = HAL_GetTick();
	  }

	  // CAN RPM
	  if((HAL_GetTick() - prevConfigTick) >= 1000)
	  {
		  uint8_t buf[8] = {0};

		  buf[0] = thruster1RPM & 0x0FF;
		  buf[1] = (thruster1RPM >> 8) & 0x0FF;
		  buf[2] = thruster2RPM & 0x0FF;
		  buf[3] = (thruster2RPM >> 8) & 0x0FF;
		  buf[4] = thruster3RPM & 0x0FF;
		  buf[5] = (thruster3RPM >> 8) & 0x0FF;
		  buf[6] = thruster4RPM & 0x0FF;
		  buf[7] = (thruster4RPM >> 8) & 0x0FF;
		  CAN_SendMsg(17, buf, 8);

		  buf[0] = thruster5RPM & 0x0FF;
		  buf[1] = (thruster5RPM >> 8) & 0x0FF;
		  buf[2] = thruster6RPM & 0x0FF;
		  buf[3] = (thruster6RPM >> 8) & 0x0FF;
		  buf[4] = thruster7RPM & 0x0FF;
		  buf[5] = (thruster7RPM >> 8) & 0x0FF;
		  buf[6] = thruster8RPM & 0x0FF;
		  buf[7] = (thruster8RPM >> 8) & 0x0FF;
		  CAN_SendMsg(18, buf, 8);

		  prevConfigTick = HAL_GetTick();
	  }

	  // CAN Status
	  if((HAL_GetTick() - prevConfigTick) >= 1000)
	  {
		  uint8_t buf[8] = {0};

		  uint16_t uptime = (HAL_GetTick() >> 9) & 0x00FF;

		  buf[7] = (uptime >> 8) & 0x00FF;
		  buf[6] = uptime & 0x00FF;
		  buf[5] = SIGNATURE;

		  CAN_SendMsg(22, buf, 8);
		  prevConfigTick = HAL_GetTick();
	  }
  }
  /* USER CODE END 3 */
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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* CEC_CAN_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CEC_CAN_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 3;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 23;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 39999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2999;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 23;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 39999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2999;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pins : PF0 PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA8 PA9 PA10
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void UpdatePWM()
{
	uint8_t Id = CAN_recvMsgBuf[CAN_RP];
	uint8_t Size = CAN_recvMsgBuf[CAN_RP + 1];
	uint8_t i = 0;
	uint16_t val = 0;
	for ( i = 0 ; i < Size ; i++){	//fill msg
				bufFifo0[i] = CAN_recvMsgBuf[CAN_RP+2+i];
	}
	if ( (Id == 1 || Id == 0) && Size != 8) {
		CAN_RP += (2 + Size);
		return;
	}
	if (Id == 0){
		for ( i = 0 ; i < Size ; i++){	//fill msg
			bufFifo0[i] = CAN_recvMsgBuf[CAN_RP+2+i];
		}
		val = (bufFifo0[1] << 8 | bufFifo0[0]) + 2499;
			// safety check
			if(val > 3499) val = 3499;
			if(val < 2499) val = 2499;
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, val);

			val = (bufFifo0[3] << 8 | bufFifo0[2]) + 2499;
			// safety check
			if(val > 3499) val = 3499;
			if(val < 2499)
			{
				val = 2499;
			}
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, val);

			val = (bufFifo0[5] << 8 | bufFifo0[4]) + 2499;
			// safety check
			if(val > 3499) val = 3499;
			if(val < 2499) val = 2499;
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, val);

			val = (bufFifo0[7] << 8 | bufFifo0[6]) + 2499;
			// safety check
			if(val > 3499) val = 3499;
			if(val < 2499) val = 2499;
			__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, val);
			frame1flag = 1;

	}
	else if (Id == 1)
	{
		val = (bufFifo0[1] << 8 | bufFifo0[0]) + 2499;
		// safety check
		if(val > 3499) val = 3499;
		if(val < 2499)
		{
			val = 2499;
		}
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, val);

		val = (bufFifo0[3] << 8 | bufFifo0[2]) + 2499;
		// safety check
		if(val > 3499) val = 3499;
		if(val < 2499)
			{
			val = 2499;
			}
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, val);

		val = (bufFifo0[5] << 8 | bufFifo0[4]) + 2499;
		// safety check
		if(val > 3499) val = 3499;
		if(val < 2499)
			{
				val = 2499;
			}
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, val);

		val = (bufFifo0[7] << 8 | bufFifo0[6]) + 2499;
		// safety check
		if(val > 3499) val = 3499;
		if(val < 2499)
			{
				val = 2499;
			}
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, val);

		frame2flag = 1;
	}

	CAN_RP += (2 + Size);

	if (CAN_full && (((CAN_RP-CAN_WP)&0x00FF) >= 20)){	//if there are at least 24 free space, continue buffer. Otherwise drop.
		CAN_full = 0;
		CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
	}


}
//
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	uint16_t val = 0;
//	// recv_databuf is library-wide data buffer
//	if (CAN_RecvMsg(CAN_RX_FIFO0, bufFifo0) == 10)
//		{
//			return;
//		};
//
//	switch(CAN_GetId()) {
//	// 1 ~ 4 Thruster Controls
//	case 0x00:
//		val = (bufFifo0[1] << 8 | bufFifo0[0]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499) val = 2499;
//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, val);
//
//		val = (bufFifo0[3] << 8 | bufFifo0[2]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499) val = 2499;
//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, val);
//
//		val = (bufFifo0[5] << 8 | bufFifo0[4]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499) val = 2499;
//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, val);
//
//		val = (bufFifo0[7] << 8 | bufFifo0[6]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499) val = 2499;
//		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, val);
//
//		frame1flag = 1;
//		break;
//
//	// 5 ~ 8 Thruster Controls
//	case 0x01:
//		val = (bufFifo0[1] << 8 | bufFifo0[0]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499)
//			{
//				val = 2499;
//			}
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, val);
//
//		val = (bufFifo0[3] << 8 | bufFifo0[2]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499)
//			{
//			val = 2499;
//			}
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, val);
//
//		val = (bufFifo0[5] << 8 | bufFifo0[4]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499)
//			{
//				val = 2499;
//			}
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, val);
//
//		val = (bufFifo0[7] << 8 | bufFifo0[6]) + 2499;
//		// safety check
//		if(val > 3499) val = 3499;
//		if(val < 2499)
//			{
//				val = 2499;
//			}
//		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, val);
//
//		frame2flag = 1;
//		break;
//
//	// Actuation Controls
//	case 0x02:
//		break;
//
//	// TAB Config
//	case 0x06:
//		break;
//
//	// General Config
//	case 0x0A:
//		break;
//
//	}
//}

//void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//	// recv_databuf is library-wide data buffer
//	CAN_RecvMsg(CAN_RX_FIFO1, bufFifo1);
//}


void CAN_RecvStart()
{
	CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_recvMsgBuf[CAN_WP] = (uint8_t) (RxHeader.StdId);	//id
	CAN_recvMsgBuf[CAN_WP+1] = (uint8_t) (RxHeader.DLC);	//len
	CAN_WP += RxHeader.DLC + 2;	//increment WP

	//if no space to receive another full length message
	if (((((CAN_RP-CAN_WP)&0x00FF) < 10)&&(CAN_RP!=CAN_WP)) || CAN_full){
		CAN_full = 1;
		return;
	}
	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
}


void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_recvMsgBuf[CAN_WP] = (uint8_t) (RxHeader.StdId);	//id
	CAN_recvMsgBuf[CAN_WP+1] = (uint8_t) (RxHeader.DLC);	//len
	CAN_WP += RxHeader.DLC + 2;	//increment WP

	//if no space to receive another full length message
	if (((((CAN_RP-CAN_WP)&0x00FF) < 10)&&(CAN_RP!=CAN_WP)) || CAN_full){
		CAN_full = 1;
		return;
	}
	CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len

}

/* USER CODE END 4 */



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */


  /* USER CODE END Error_Handler_Debug */
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
