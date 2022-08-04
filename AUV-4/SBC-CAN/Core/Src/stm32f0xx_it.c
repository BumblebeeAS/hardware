/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f0xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#include "stm32f0xx_it.h"

extern TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart2;
extern msg_queue uart_txbuf, uart_rxbuf;
extern volatile uint8_t isUartIdle;
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern CAN_HandleTypeDef hcan;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */

	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */

	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
	/* USER CODE BEGIN HardFault_IRQn 0 */

	/* USER CODE END HardFault_IRQn 0 */
	while (1) {
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
	/* USER CODE BEGIN SVC_IRQn 0 */

	/* USER CODE END SVC_IRQn 0 */
	/* USER CODE BEGIN SVC_IRQn 1 */

	/* USER CODE END SVC_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
	/* USER CODE BEGIN PendSV_IRQn 0 */

	/* USER CODE END PendSV_IRQn 0 */
	/* USER CODE BEGIN PendSV_IRQn 1 */

	/* USER CODE END PendSV_IRQn 1 */

}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
	/* USER CODE BEGIN SysTick_IRQn 0 */

	/* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */

	/* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles HDMI-CEC and CAN global interrupts / HDMI-CEC wake-up interrupt through EXTI line 27.
 */
void CEC_CAN_IRQHandler(void) {

	HAL_CAN_IRQHandler(&hcan);

}

void TIM17_IRQHandler(void) {

}

void USART2_IRQHandler(void) {
	// static uint32_t cnt = 0;
	// cnt ++;

	// multiple interrupts should be cleared in a single call
	// HAL_NVIC_ClearPendingIRQ(USART3_IRQn);

	// RXNE
	if (huart2.Instance->ISR & (1 << 5))
	{
		uint8_t d = huart2.Instance->RDR;
		if (huart2.Instance->ISR & (1 << 2 ) || huart2.Instance->ISR & (1 << 1) || huart2.Instance->ISR & (1 << 0) || !isUartIdle)
			return;
		MsgQueue_push(&uart_rxbuf,d);
	}

	// Overrun error
	if (huart2.Instance->ISR & (1 << 3))
	{
		// clear ORE
		huart2.Instance->ICR |= (1 << 3);
		uint8_t d = huart2.Instance->RDR;

		if (huart2.Instance->ISR & (1 << 2 ) || huart2.Instance->ISR & (1 << 1) || huart2.Instance->ISR & (1 << 0) || !isUartIdle)
			return;
		MsgQueue_push(&uart_rxbuf,d);
	}

//	// TC
//	if (huart2.Instance->SR & (1 << 6))
//	{
//		huart2.Instance->SR &= ~(1 << 6);
//		return;
//	}
	// TXE
	if (huart2.Instance->ISR & (1 << 7))
	{
		if (MsgQueue_Empty(&uart_txbuf) && (huart2.Instance->ISR & (1 << 6)))
		{
			// disable TXEIE
			huart2.Instance->CR1 &= ~(1 << 7);
		}
		else if ( !MsgQueue_Empty(&uart_txbuf) )
		{
			huart2.Instance->TDR = MsgQueue_pop(&uart_txbuf);
		}
	}
}

