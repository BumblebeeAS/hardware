#include "peripheral_Init.h"

/***************Project Specific Functions**************/


extern uint32_t Error_Status;
/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
//void MX_SPI1_Init(void)
//{
//
//  hspi1.Instance = SPI1;
//  hspi1.Init.Mode = SPI_MODE_MASTER;
//  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
//  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi1.Init.NSS = SPI_NSS_SOFT;
//  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
//  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//  hspi1.Init.CRCPolynomial = 7;
//  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
//  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
//  if (HAL_SPI_Init(&hspi1) != HAL_OK)
//  {
//	  Error_Status |= 1 << SPI_INIT_ERROR;
//	  Error_Handler();
//  }
//}




/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
//Use basic mode for TIM17
void MX_TIM17_Init(void)
{
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 24;  //25M / (24+1) = 1MHz
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 999;	//1M / 1000 = 1kHz
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim17.Init.RepetitionCounter = 0x0000;	//generate interrupt after n clock overflow/underflow
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init (&htim17) != HAL_OK)
  {
//	  Error_Status |= 1 << TIM17_INIT_ERROR;
    Error_Handler();
  }
}




/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{
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
//	  Error_Status |= 1 << UART1_INIT_ERROR;
    Error_Handler();
  }
}


/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART2_UART_Init(void)
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
//	  Error_Status |= 1 << UART2_INIT_ERROR;
	  Error_Handler();
  }
}


// Use GPIO from main
///**
//  * @brief GPIO Initialization Function
//  * @param None
//  * @retval None
//  */
//
//void MX_GPIO_Init(void)
//{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//  /* GPIO Ports Clock Enable */
//  __HAL_RCC_GPIOF_CLK_ENABLE();
//  __HAL_RCC_GPIOA_CLK_ENABLE();
//  __HAL_RCC_GPIOB_CLK_ENABLE();
//
//  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_4|GPIO_PIN_15, GPIO_PIN_RESET);
//
//  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
//                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);
//
//  /*Configure GPIO pin : PF1 */
//  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
//
//
//
//  //power control pins
//  //***********************//
//  /*Configure GPIO pins : PA15 */
//  GPIO_InitStruct.Pin = GPIO_PIN_15;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//  /*Configure GPIO pins : PB0 PB1 PB3 PB4
//                           PB5 PB6 PB7 */
//  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4
//                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
//  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//  //***********************//
//
//
//
//  /*Configure GPIO pins : PA1 PA8 */
//  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_8;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//
//
//  /*Configure GPIO pin : PB8 */
//  GPIO_InitStruct.Pin = GPIO_PIN_8;
//  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//}


