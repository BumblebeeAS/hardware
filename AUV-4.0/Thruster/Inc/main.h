/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "Thrusters.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define T2_PWM_ISO_Pin GPIO_PIN_0
#define T2_PWM_ISO_GPIO_Port GPIOA
#define T1_PWM_ISO_Pin GPIO_PIN_1
#define T1_PWM_ISO_GPIO_Port GPIOA
#define T3_PWM_ISO_Pin GPIO_PIN_2
#define T3_PWM_ISO_GPIO_Port GPIOA
#define T4_PWM_ISO_Pin GPIO_PIN_3
#define T4_PWM_ISO_GPIO_Port GPIOA
#define T8_PWM_ISO_Pin GPIO_PIN_6
#define T8_PWM_ISO_GPIO_Port GPIOA
#define T7_PWM_ISO_Pin GPIO_PIN_7
#define T7_PWM_ISO_GPIO_Port GPIOA
#define T6_PWM_ISO_Pin GPIO_PIN_0
#define T6_PWM_ISO_GPIO_Port GPIOB
#define T5_PWM_ISO_Pin GPIO_PIN_1
#define T5_PWM_ISO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
