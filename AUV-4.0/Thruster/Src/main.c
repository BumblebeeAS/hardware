
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "retarget.h"
#include <stdio.h>
#include "can_lib.h"
#include "can_defines.h"

#define VERB 0 //verbose mode for CAN

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);

static void MX_USART2_UART_Init(void);

/* CAN Variables */
uint8_t data_recv[8];

/* Thruster Variables */
Thruster thruster1;
Thruster thruster2;
Thruster thruster3;
Thruster thruster4;
Thruster thruster5;
Thruster thruster6;
Thruster thruster7;
Thruster thruster8;
static uint8_t thruster_enable = 0;
short thruster_speed_buf[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //Speed Change buffer for Thrusters

/* Timing Variables */
static uint32_t currentTick;
static uint32_t thruster_loop_tick; //20ms loop for thrusters
static uint32_t hb_sync_timer = 0;
static uint32_t hb_local_timer = 0;
static uint32_t heartbeat_loop = 0;
#define HEARTBEAT_TIMEOUT 500

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  //MX_USART1_UART_Init();
  MX_TIM14_Init();
  //RetargetInit(&huart1);

  MX_USART2_UART_Init(); //Uart2 init
  RetargetInit(&huart2);

  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  ThrusterInit(&thruster2, 2, 0, &htim2, TIM_CHANNEL_1);
  ThrusterInit(&thruster1, 2, 0, &htim2, TIM_CHANNEL_2);
  ThrusterInit(&thruster3, 2, 0, &htim2, TIM_CHANNEL_3);
  ThrusterInit(&thruster4, 2, 0, &htim2, TIM_CHANNEL_4);
  ThrusterInit(&thruster5, 2, 0, &htim2, TIM_CHANNEL_4);
  ThrusterInit(&thruster6, 2, 0, &htim2, TIM_CHANNEL_3);
  ThrusterInit(&thruster7, 2, 0, &htim2, TIM_CHANNEL_2);
  ThrusterInit(&thruster8, 2, 0, &htim2, TIM_CHANNEL_1);

  CAN_Begin();
  uint8_t data_send[8] = {0x31,0x32,0x33,0x34,0x31,0x32,0x33,0x34};
  uint8_t data_send_heartbeat[1] = {0x1};

  while (1)
  {
    /*** Testing UART	   */
    //		uint8_t data[8];
    //		HAL_UART_Transmit(&huart1, data, 8, HAL_MAX_DELAY);
    //		printf("UART 1 is working");

    /*** Testing UART	End  */

    /*** Testing CAN	   */
    CAN_SendMsg(CAN_thruster_1, data_send); //id, len, data buf
//    CAN_SendMsg(CAN_heartbeat, data_send_heartbeat);
    HAL_Delay(500);
    checkCANmsg();
    //id ++;
    // moved to interrupt callback
    //FIFO, data buf

    /*** Testing CAN	End  */

    /*** Testing Thrusters	End  */
    //		printf("Current Thruster PWM is");
    //		printf(": %d", thruster2.current_pwm);
    //		HAL_Delay(2000);
    //
    //		printf("Calling mov thrusters \n");
    //		mov(&thruster2, -2600); //works now but range abit messed up
    //		printf("Current Thruster PWM is");
    //		printf(": %d", thruster2.current_pwm);
    //		HAL_Delay(2000);

    /*** Testing Thrusters	End  */

    /*****************************************/
    /*  Heartbeat							 */
    /*  Maintain comms with SBC				 */
    /*****************************************/
    //If 500ms have passed, run this section
	if((HAL_GetTick() - heartbeat_loop) > HEARTBEAT_TIMEOUT){
		//CAN.setupCANFrame(hb_buf, 0, 1, HEARTBEAT_TB);
		//CAN.sendMsgBuf(CAN_heartbeat, 0, 1, hb_buf);
		hb_local_timer = HAL_GetTick();

		//If 3000ms have passed since SBC HB have been received, disable thrusters
		if(hb_local_timer - hb_sync_timer > 3000){
			//No SBC HB
			printf("Thruster Enable changed to 0 \n");
			thruster_enable = 0;
		} else {
			printf("Thruster Enabled! \n");
			thruster_enable = 1;
		}
		heartbeat_loop = HAL_GetTick();
	}
	currentTick = HAL_GetTick();

    /*****************************************/
    /*  Heartbeat							 */
    /*  Maintain comms with SBC				 */
    /*****************************************/

    //		if(currentTick > thruster_loop_tick + 20 || currentTick < thruster_loop_tick){
    //			if (!thruster_enable) {
    //				// stop thrusters
    //				Thruster_buf[0] = 0;
    //				Thruster_buf[1] = 0;
    //				Thruster_buf[2] = 0;
    //				Thruster_buf[3] = 0;
    //				Thruster_buf[4] = 0;
    //				Thruster_buf[5] = 0;
    //				Thruster_buf[6] = 0;
    //				Thruster_buf[7] = 0;
    //			}
    //
    //			for (int i = 0; i < 8; i++)
    //			{
    //				printf(i);
    //				printf(": ");
    //				printf(Thruster_buf[i]);
    //				printf("\t");
    //			}
    //			printf("run");
    //			mov(&thruster1, thruster_speed_buf[0]);
    //			mov(&thruster2, thruster_speed_buf[1]);
    //			mov(&thruster3, thruster_speed_buf[2]);
    //			mov(&thruster4, thruster_speed_buf[3]);
    //			mov(&thruster5, thruster_speed_buf[4]);
    //			mov(&thruster6, thruster_speed_buf[5]);
    //			mov(&thruster7, thruster_speed_buf[6]);
    //			mov(&thruster8, thruster_speed_buf[7]);
    //			thruster_loop_tick = __HAL_TIM_GET_COUNTER(&htim14);
    //		}
  }
}

void checkCANmsg()
{
  short thruster_speed;
  //If RetVal of function is 10, no messages
  if (CAN_RecvMsg(CAN_RX_FIFO0, data_recv, &RxHeader) != 10)
  {
    switch (RxHeader.StdId)
    {
    case CAN_thruster_1:
      printf("CAN_thruster_1 ID \n");
      for (int i = 0; i < 4; i++)
      {
        printf(i);
        printf(": ");
        thruster_speed = (data_recv[i * 2] & 0xff) | ((data_recv[(i * 2) + 1] & 0xff) << 8);
        thruster_speed_buf[i] = thruster_speed - 3200;
        printf("%d",thruster_speed_buf[i]);
        printf("\t");
      }
      printf("");
      break;

    case CAN_thruster_2:
      printf("CAN_thruster_2 ID  \n");
      for (int i = 0; i < 4; i++)
      {
        printf(i + 4);
        printf(": ");
        thruster_speed = (data_recv[i * 2] & 0xff) | ((data_recv[(i * 2) + 1] & 0xff) << 8);
        thruster_speed_buf[i + 4] = thruster_speed - 3200;
        printf(thruster_speed_buf[i + 4]);
        printf("\t");
      }
      printf("");
      break;

    default:
      printf("Unknown CAN message received");
      break;
    }
  }
  else if (CAN_RecvMsg(CAN_RX_FIFO1, data_recv, &RxHeader) != 10)
  {
    switch (RxHeader.StdId)
    {
    case CAN_heartbeat:
      if (data_recv[0] == HEARTBEAT_SBC)
      {
        printf("CAN heartbeat");
        hb_sync_timer = __HAL_TIM_GET_COUNTER(&htim14); //Each increment is a millisecond
        printf("HB Sync Timer Tick: %d", hb_sync_timer);
      }
      break;

    case CAN_ST_stats:
      printf("id-");
      printf("CAN_ST_stats");
      printf(": ");
      for (int i = 0; i < 8; i++)
      {
        printf(data_recv[i]);
        printf(" ");
      }
      printf("");
      if (data_recv[3] == 1)
      {
        printf("Leaking");
        while (1)
        {
          mov(&thruster1, thruster_speed_buf[0]);
          mov(&thruster2, thruster_speed_buf[1]);
          mov(&thruster3, thruster_speed_buf[2]);
          mov(&thruster4, thruster_speed_buf[3]);
          mov(&thruster5, thruster_speed_buf[4]);
          mov(&thruster6, thruster_speed_buf[5]);
          mov(&thruster7, thruster_speed_buf[6]);
          mov(&thruster8, thruster_speed_buf[7]);
        }
      }
      break;
    default:
      //TODO=>throw an error
      printf("Unknown CAN message received");
      break;
    }
  }
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
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
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
//static void MX_CAN_Init(void)
//{
//  hcan.Instance = CAN;
//  hcan.Init.Prescaler = 16;
//  hcan.Init.Mode = CAN_MODE_NORMAL;
//  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
//  hcan.Init.TimeSeg1 = CAN_BS1_1TQ;
//  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
//  hcan.Init.TimeTriggeredMode = DISABLE;
//  hcan.Init.AutoBusOff = DISABLE;
//  hcan.Init.AutoWakeUp = DISABLE;
//  hcan.Init.AutoRetransmission = DISABLE;
//  hcan.Init.ReceiveFifoLocked = DISABLE;
//  hcan.Init.TransmitFifoPriority = DISABLE;
//  if (HAL_CAN_Init(&hcan) != HAL_OK)
//  {
//    Error_Handler();
//  }
//}

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
  htim2.Init.Prescaler = 95;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10000;
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
  sConfigOC.Pulse = 0;
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
  htim3.Init.Prescaler = 95;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10000;
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
  sConfigOC.Pulse = 0;
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
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 0;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */
  HAL_TIM_Base_Start(&htim14);
  /* USER CODE END TIM14_Init 2 */
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
  huart1.Init.BaudRate = 38400;
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
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (huart->Instance == USART1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX --> D5
    PB7     ------> USART1_RX --> D4
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Left as no pull
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }

  if (huart->Instance == USART2)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
      PA2     ------> USART2_TX
      PA3     ------> USART2_RX
      */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX --> D5
    PB7     ------> USART1_RX --> D4
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
  }

  if (huart->Instance == USART2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
     PA2     ------> USART2_TX
     PA3     ------> USART2_RX
     */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
  }
}

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

#ifdef USE_FULL_ASSERT
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
