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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "screen.h"
#include "sensor.h"
#include "st_can.h"
#include "can_lib.h"
#include "stdlib.h"

uint8_t* CAN_recvMsgBuf;
uint8_t CAN_WP = 0;
uint8_t CAN_RP = 0;
uint8_t CAN_full = 0;
uint8_t CAN_Last_RP = 0;	//use to see if new msg is coming in


uint32_t can_publish_loop_gyro=10;
uint32_t can_publish_loop_mag=10;
uint32_t can_publish_loop_acc=10;
uint32_t can_publish_loop_sens=10;
uint32_t can_publish_loop_uptime=0;
uint32_t check_hb_loop=0;

uint32_t can_heartbeat_loop=0;
uint8_t CAN_SEND=0;
uint16_t uptime=0;
uint8_t signature=0;

uint8_t CAN_msg[16];

uint8_t CAN_overflow=0;

//CAN receive buffer


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


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
  HAL_Delay(1);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();

  //initialize CAN recvBuf
	do{
		CAN_recvMsgBuf = (uint8_t*) malloc(256*sizeof(uint8_t));
	} while (CAN_recvMsgBuf == NULL);

	uint32_t i = 0;
	for (i = 0 ; i < 256 ; i++){
		CAN_recvMsgBuf[i] = 0;
	}

	for (i = 0; i<=8; i++){
		boardHB_timeout[i]=0;
	}

	for (i = 0; i <8; i++){
		stbUP[i] = 1;
	}

	for (i=0;i<=15;i++){
		CAN_msg[i]=0;
	}
  /* USER CODE BEGIN 2 */
  begin();
  displayOn();
  GPIOX();
  PWM1Config();
  PWM1out();
  fillScreen(0x0000);
  textMode();
  textTransparent(0x0000);
  textenlarge(1);
  HAL_Delay(1000);

  screen_default();
  default_values();
  update_screen();
  check_hb();
  HAL_Delay(300);
  CAN_Begin(CAN_MODE_NORMAL);
  CAN_SetAllFilters();
  CAN_RecvStart();

//  init_LSM6();
//  init_LIS3();
//  ExtPress_init();

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */
		  publishCAN_message();
		  publishCAN_heartbeat();
		  publishCAN_uptime();
		  CAN_routine();
		  //sensor_update();
	//	  IntPressure_read();
	//	  LIS3_read();
	//	  LSM6_read();
	//	  ExtPress_read();
		  update_screen();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
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
  /* USER CODE BEGIN CAN_Init 2 */
  void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
  {
	//CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
  	CAN_recvMsgBuf[CAN_WP] = (uint8_t) (RxHeader.StdId);	//id
  	CAN_recvMsgBuf[CAN_WP+1] = (uint8_t) (RxHeader.DLC);	//len
  	CAN_WP += RxHeader.DLC + 2;	//increment WP RxHeader.DLC +

  	//if no space to receive another full length message
  	if (((((CAN_RP-CAN_WP)&0x00FF) < 10)&&(CAN_RP!=CAN_WP)) || CAN_full){
  		CAN_full = 1;
  		return;
  	}
  	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
  }


  void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
  {
	//CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
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

  void CAN_RecvStart()
  {
  	CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
  	CAN_RecvMsg(CAN_RX_FIFO0,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
  }


  void CAN_routine(void){
  		while ( ((CAN_WP-CAN_RP)&0x00FF) > 20 ){
  			uint8_t Id = CAN_recvMsgBuf[CAN_RP];
  			uint8_t Size = CAN_recvMsgBuf[CAN_RP + 1];
  			uint8_t i = 0;
  			for ( i = 0 ; i < Size ; i++){
  						CAN_msg[i] = CAN_recvMsgBuf[CAN_RP + 2 + i];
  					}


  						switch(Id){
  						case CAN_HEARTBEAT:{
  							uint8_t device= CAN_msg[0];
  							boardHB_timeout[device]=HAL_GetTick();
  							break;
  						}
  						case CAN_BATT1_STAT:{
  							powerStats[BATT1_CURRENT]=(CAN_msg[1]<<8|CAN_msg[0]);
  							powerStats[BATT1_VOLTAGE]=(CAN_msg[3]<<8|CAN_msg[2]);
  							break;
  						}
  						case CAN_PMB1_STAT:{
  							internalStats[PMB1_PRESS]=CAN_msg[4];
  							internalStats[PMB1_TEMP]=CAN_msg[3];
  							powerStats[BATT1_CAPACITY]=CAN_msg[2];
  							break;
  						}
  						case CAN_BATT2_STAT:{
  							powerStats[BATT2_CURRENT]=(CAN_msg[1]<<8|CAN_msg[0]);
  							powerStats[BATT2_VOLTAGE]=(CAN_msg[3]<<8|CAN_msg[2]);
  							break;
  						}
  						case CAN_PMB2_STAT:{
  							internalStats[PMB2_PRESS]=CAN_msg[4];
  							internalStats[PMB2_TEMP]=CAN_msg[3];
  							powerStats[BATT2_CAPACITY]=CAN_msg[2];
  							break;
  						}
  						case CAN_STB_CONFIG:{
  							for (uint8_t i=0;i<STB_UP_COUNT;i++){
  								stbUP[i]=CAN_msg[i];
  							}
  							set_led(stbUP[LED]);
  							uint8_t MSG[8]={stbUP[7],stbUP[6],stbUP[5],stbUP[4],stbUP[3],stbUP[2],stbUP[1],stbUP[0]};
  							CAN_SendMsg(CAN_STB_UP,MSG,8);
  							break;
  						}
  						default:
  							break;
  						}
  						CAN_RP = CAN_RP+2+Size;

  					}

  				//reset CAN_full flag is more than 20 bytes free space
  				if (CAN_full && (((CAN_RP-CAN_WP)&0x00FF) >= 20)){	//if there are at least 24 free space, continue buffer. Otherwise drop.
  					CAN_full = 0;
  					CAN_RecvMsg(CAN_RX_FIFO1,CAN_recvMsgBuf+CAN_WP+2);	//save 1 space for ID and 1 for len
  				}

  					//update screen
  					if(HAL_GetTick()-check_hb_loop>1000){
  					for (uint8_t i=1;i<=6;i++){
  						if((HAL_GetTick()-boardHB_timeout[i])>3000){
  						boardHB[i-1]=0xFFFF;
  						}
  					else{
  						boardHB[i-1]=0x1111;
  						}
  					}
//  					for (uint8_t i=6;i<9;i++){
//  						if((HAL_GetTick()-boardHB_timeout[i])>HB_TIMEOUT){
//  						boardHB[i-1]=0xFFFF;
//  						}
//  					else{
//  						boardHB[i-1]=0x1111;
//  						}
//  					}
  					check_hb_loop=HAL_GetTick();
  					}
  }


  void publishCAN_message(void){
  	if(HAL_GetTick()-can_publish_loop_gyro>(1000/stbUP[GYRO])){
  	uint8_t Msg[6]={(uint8_t)internalStats[IMU_G_X]>>8,(uint8_t)internalStats[IMU_G_X],(uint8_t)internalStats[IMU_G_Y]>>8,(uint8_t)internalStats[IMU_G_Y],(uint8_t)internalStats[IMU_G_Z]>>8,(uint8_t)internalStats[IMU_G_Z]};
  	CAN_SendMsg(CAN_STB_GYRO,Msg,6);
  	can_publish_loop_gyro=HAL_GetTick();
  	}

  	if(HAL_GetTick()-can_publish_loop_gyro>(1000/stbUP[ACC])){
  		uint8_t Msg[6]={(uint8_t)internalStats[IMU_A_X]>>8,(uint8_t)internalStats[IMU_A_X],(uint8_t)internalStats[IMU_A_Y]>>8,(uint8_t)internalStats[IMU_A_Y],(uint8_t)internalStats[IMU_A_Z]>>8,(uint8_t)internalStats[IMU_A_Z]};
  		CAN_SendMsg(CAN_STB_ACC,Msg,6);
  		can_publish_loop_acc=HAL_GetTick();
  	}

  		if(HAL_GetTick()-can_publish_loop_mag>(1000/stbUP[MAG])){
  		uint8_t Msg[6]={internalStats[IMU_M_X]>>8,internalStats[IMU_M_X],internalStats[IMU_M_Y]>>8,internalStats[IMU_M_Y],internalStats[IMU_M_Z]>>8,internalStats[IMU_M_Z]};
  		CAN_SendMsg(CAN_STB_MAG,Msg,6);
  		can_publish_loop_mag=HAL_GetTick();
  		}

  	if(HAL_GetTick()-can_publish_loop_sens>(1000/stbUP[SENSOR])){
  	uint8_t Msg2[8]={internalStats[INT_PRESS]>>8,internalStats[INT_PRESS],internalStats[HUMIDITY]>>8,internalStats[HUMIDITY],internalStats[ST_TEMP]>>8,internalStats[ST_TEMP],internalStats[EXT_PRESS]>>8,internalStats[EXT_PRESS]};
  	CAN_SendMsg(CAN_STB_SENS,Msg2,8);
  	can_publish_loop_sens=HAL_GetTick();
  	}
  	}

  void publishCAN_heartbeat(void){
  	if( (HAL_GetTick()-can_heartbeat_loop)>500){
  		uint8_t Msg[1]={HEARTBEAT_STB};
  		CAN_SendMsg(CAN_HEARTBEAT,Msg,1);
  		can_heartbeat_loop=HAL_GetTick();
  	}
  }

  void publishCAN_uptime(void){
  	if((HAL_GetTick()-can_publish_loop_uptime)>1000){
  		uptime=uptime+1;
  		uint8_t MSG[3]={uptime>>8,uptime,signature};
  		CAN_SendMsg(CAN_STB_STAT,MSG,3);
  		can_publish_loop_uptime=HAL_GetTick();
  	}
  }

  /* USER CODE END CAN_Init 2 */


/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing =0x2000090E; //0x20303E5D;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter 
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter 
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 PC15 PC0 
                           PC1 PC2 PC3 PC4 
                           PC6 PC7 PC8 PC9 
                           PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0 
                          |GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4 
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA7 
                           PA8 PA9 PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC5 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 
                           PB11 PB12 PB13 PB14 
                           PB15 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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
