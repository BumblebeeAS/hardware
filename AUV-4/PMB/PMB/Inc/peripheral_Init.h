#ifndef _PERIPHERAL_INIT_H
#define _PERIPHERAL_INIT_H

#include "stm32f0xx_hal.h"
#include "main.h"

/***************Struct Declaration**************/

uint32_t TIM_Tick;
uint32_t counter;

//SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim17;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern CAN_HandleTypeDef hcan; 			//hcan struct


/***************Function Prototypes**************/
//void MX_GPIO_Init(void);
//void MX_SPI1_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_TIM17_Init(void);

#endif
