#include "stdio.h"
#include "stdlib.h"
#include "main.h"
#include "pmb_define.h"
#include "ADS1115.h"
#include "BQ34110.h"
#include "INA233.h"

// CAN
#include "Error_Define.h"
#include "can_lib.h"
#include "peripheral_Init.h"
#include "retarget.h"

// SSD
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"

#include <string.h>

CAN_HandleTypeDef hcan;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint32_t Error_Status;
void System_Begin();
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
void CAN_SetAllFilters();
void ssd1306_DisplayData();
void setState(uint16_t status);
void ssd1306_DisplayOffMsg();
void ssd1306_DisplayOnMsg();

uint32_t RxFifo = 10;

//Timer tick
uint32_t TIM_Tick = 0;

//take in 1500 msgs, 1000 * 3 ~= 3100
// Pos 	0			1			2
// 		Time(low)	Time(high) 	Id
uint8_t TrafficLog[3100] = {0};
uint16_t log_cnt = 0;

// pos	0			1				2				3										4					5
//		Id_cnt		Last tick(low)	Last tick(High)	Duration(low)(used for freq estimate) 	Duration(High)		Diagnose
//uint8_t TrafficReport[250] = {0};	//num of id ~= 36 => 36*6

uint16_t TrafficReport[150] = {0};
uint32_t tick_5hz = 0;
uint32_t tick_1hz = 0;
uint32_t tick_hb = 0;
uint8_t hb[1] = {0};
uint16_t status = 0;

float current;
float board_temperature;
float board_pressure = 0;
//uint16_t current, board_temperature, board_pressure;
uint16_t voltage;
uint16_t soc, fcc, consumption;
uint32_t update_timer = 0;

float ina_current = -1;

char state[15] = "";

int main(void)
{
	System_Begin();

	int counter = 0;

//	printf("Hi! I am CAN PMB AUV4.0!\r\n");

	uint8_t batt_msgbuf[8] = {50,51,52,53,54,55,56,57};
	uint8_t pmb_msgbuf[8] = {50,51,52,53,54,55,56,57};
	HAL_StatusTypeDef stat1 = HAL_ERROR;
	HAL_StatusTypeDef stat2 = HAL_ERROR;
	HAL_StatusTypeDef prev_stat3 = HAL_ERROR;
	HAL_StatusTypeDef stat3 = HAL_ERROR;

	HAL_GPIO_WritePin(PORT_PMOS, PIN_PMOS, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PORT_RELAY, PIN_RELAY, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PORT_ALERT1, PIN_ALERT1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PORT_ALERT2, PIN_ALERT2, GPIO_PIN_RESET);

	ssd1306_Init();
	ssd1306_DisplayOnMsg();

	prev_stat3 = HAL_I2C_IsDeviceReady(&hi2c1, SSD1306_I2C_ADDR, 10, 10);

//	BQ_Init();
//	BQ_CalibrateVoltage(16130);
//	BQ_CalibrateCurrent(200);

//	BQ_Learning();
//	BQ_RestoreCCSettings();

//	ssd1306_TestAll();

//	INA_Init();

	while (1) {
	  counter = counter + 1;
	  status = BQ_GetBattStatus();
	  // TEST POWER CONTROL
//	  HAL_GPIO_WritePin(PORT_PMOS, PIN_PMOS, GPIO_PIN_SET);
//	  HAL_GPIO_WritePin(PORT_RELAY, PIN_RELAY, GPIO_PIN_RESET);
	  if (HAL_GPIO_ReadPin(PORT_OFF, PIN_OFF)) {
		  while(1) {
			  HAL_GPIO_WritePin(PORT_PMOS, PIN_PMOS, GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(PORT_RELAY, PIN_RELAY, GPIO_PIN_SET);
		  }
	  }

	  /* TEST DEVICES
	  stat1 = HAL_I2C_IsDeviceReady(&hi2c1, BQ34110_ADDRESS,10,10);
	  stat2 = HAL_I2C_IsDeviceReady(&hi2c1, ADS1115_ADDRESS, 10, 10);
	  */
	  prev_stat3 = stat3;
	  stat3 = HAL_I2C_IsDeviceReady(&hi2c1, SSD1306_I2C_ADDR, 10, 10);

	  if (prev_stat3 == HAL_ERROR && stat3 == HAL_OK) {
		  ssd1306_Init();
	  }

	  // Alerts
//	  if (soc <= 20) {
//		  HAL_GPIO_WritePin(PORT_ALERT1, PIN_ALERT1, GPIO_PIN_SET);
//	  }
//	  else if (voltage <= 14910) {
//		  HAL_GPIO_WritePin(PORT_ALERT2, PIN_ALERT2, GPIO_PIN_SET);
//	  }
//	  else {
//		  HAL_GPIO_WritePin(PORT_ALERT1, PIN_ALERT1, GPIO_PIN_RESET);
//		  HAL_GPIO_WritePin(PORT_ALERT2, PIN_ALERT2, GPIO_PIN_RESET);
//	  }

//	   * CAN
	  if (HAL_GetTick() - update_timer > 50) {
		  update_timer = HAL_GetTick();
		  current = BQ_GetCurrent();
		  voltage = BQ_GetVoltage();
		  consumption = BQ_GetRemainingCapacity();
		  soc = BQ_GetRSOC();
		  board_pressure = (ADS_ReadADC_SingleEnded(1)*0.0001875) / (INTPRES_REF * 0.0040) + 10;
		  board_temperature = (float)BQ_GetTemp();

		  for (int i=0; i<8; i++) {
			  batt_msgbuf[i] = i;
		  }

		  batt_msgbuf[0] = (uint8_t)((uint16_t)current & 0xFF);
		  batt_msgbuf[1] = (uint8_t)((uint16_t)current >> 8);
		  batt_msgbuf[2] = (uint8_t)(voltage & 0xFF);
		  batt_msgbuf[3] = (uint8_t)(voltage >> 8);
		  batt_msgbuf[4] = (uint8_t)(consumption & 0xFF);
		  batt_msgbuf[5] = (uint8_t)(consumption >> 8);
		  batt_msgbuf[6] = (uint8_t)(soc & 0xFF);
		  batt_msgbuf[7] = (uint8_t)(soc >> 8);

		  for (int i=0; i<2; i++) {
			  pmb_msgbuf[i] = i;
		  }

		  pmb_msgbuf[0] = (int8_t)((int)board_temperature & 0xFF);
		  pmb_msgbuf[1] = (int8_t)((int)board_temperature >> 8);
		  pmb_msgbuf[2] = (uint8_t)((uint16_t)board_pressure & 0xFF);
		  pmb_msgbuf[3] = (uint8_t)((uint16_t)board_pressure >> 8);
	  }

	  // TEST CAN
	  if ((HAL_GetTick() - tick_5hz) > CAN_STATUS_INTERVAL){
		  tick_5hz = HAL_GetTick();
		  CAN_SendMsg(CAN_BATT_STAT_ID, batt_msgbuf, 8);
	  }
	  if ((HAL_GetTick() - tick_1hz) > CAN_STATUS_INTERVAL){
		  tick_1hz = HAL_GetTick();
		  CAN_SendMsg(CAN_PMB_STAT_ID, pmb_msgbuf, 8);
	  }
	  if ((HAL_GetTick() - tick_hb) > CAN_HEARTBEAT_INTERVAL){
	 	  tick_hb = HAL_GetTick();
	 	  hb[0] = PMB_HEARTBEAT_ID;
	 	  CAN_SendMsg(4,hb,1);
	  }

	  ssd1306_DisplayData();
  }
}

void ssd1306_DisplayData() {
	char buff[64];
	uint16_t integer, fraction;
	ssd1306_Fill(White);
	ssd1306_SetCursor(2,1);
	ssd1306_WriteString("Temp:    ", Font_6x8, Black);
    snprintf(buff, sizeof(buff), "%d C", (int)board_temperature);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(96,1);
	ssd1306_WriteString("PMB ", Font_6x8, Black);
    snprintf(buff, sizeof(buff), "%d", PMB_NO);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,11);
	ssd1306_WriteString("Pres:    ", Font_6x8, Black);
	integer = (int) board_pressure;
	fraction = (int)((board_pressure - integer)*100);
    snprintf(buff, sizeof(buff), "%d.%02d kPa", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,21);
	ssd1306_WriteString("Current: ", Font_6x8, Black);
	integer = (int)(current/1000);
	fraction = (int)((current/1000.0 - integer)*100);
    snprintf(buff, sizeof(buff), "%d.%02d A", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("Voltage: ", Font_6x8, Black);
	integer = (int)(voltage/1000);
	fraction = (int)((voltage/1000.0 - integer)*100);
    snprintf(buff, sizeof(buff), "%d.%02d V", integer, fraction);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,41);
	ssd1306_WriteString("Batt %:  ", Font_6x8, Black);
    snprintf(buff, sizeof(buff), "%d", (int)soc);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_SetCursor(2,51);
	ssd1306_WriteString("State:   ", Font_6x8, Black);
	setState(status);
    snprintf(buff, sizeof(buff), "%s", state);
    ssd1306_WriteString(buff, Font_6x8, Black);

    ssd1306_UpdateScreen();
}

void ssd1306_DisplayOnMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("TURNING ON PMB ...", Font_6x8, Black);
	HAL_Delay(1000);
}

void ssd1306_DisplayOffMsg() {
	ssd1306_Fill(White);
    ssd1306_SetCursor(2,31);
	ssd1306_WriteString("TURNING OFF PMB ...", Font_6x8, Black);
	HAL_Delay(1000);
}

void setState(uint16_t status) {
	if (status & 0x02) {
		snprintf(state, sizeof(state), "CHARGING");
	}
	else if (status & 0x01){
		snprintf(state, sizeof(state), "DISCHARGING");
	}
	else {
		snprintf(state, sizeof(state), "DISCHARGING");
	}
}

void CAN_SetAllFilters()
{
	/* ID 0 ~ 15 goes to FIFO0
	 * Filter configuration:
	 * ID: 		0b00001111
	 * Mask:	0b11110000
	 * Effect:  Accept all IDs below 16
	 */
	CAN_SetFilter(0x0F, 0xF0, 0x0F, 0xF0, 1, CAN_RX_FIFO0);

	/* ID 16 ~  goes to FIFO1
	 * Filter configuration:
	 * ID: 		0x00
	 * Mask:	0x00
	 * Effect:  Accept all. But since 0~15 will go into filter bank with higher priority,
	 * only 16~ will go here
	 */
	CAN_SetFilter(0x00, 0x00, 0x00, 0x00, 2, CAN_RX_FIFO1);
}

//enable NVIC, reset TIM_Tick and starts TIM17 (base mode)
void Enable_TIM17(void)
{
	HAL_NVIC_SetPriority(TIM17_IRQn,3,0);
	HAL_NVIC_EnableIRQ(TIM17_IRQn); 	//enable timer
	TIM_Tick = 0;						//reset TIM_Tick
	HAL_TIM_Base_Start_IT(&htim17);
}

void System_Begin()
{

	//Peripheral inits are in the peripheral_Init.c
	uint32_t Error_Status = 0;	        //reset error status
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_TIM17_Init();

	//hackjob to solve mysterious reset issue caused by PA0 and PA4(?)
	OB_TypeDef Res;
	Res.USER |= ((1 << 1) | (1 << 2));	//disable stop mode and sleep mode
	// RCC_TypeDef RCC_S;
	// RCC_S.CSR;	 //disable reset upon stop /sleep mode
	  	  	  	  	 //This register report why chip reseted

	SysTick_Config(SystemCoreClock/1000);
	RetargetInit(&huart2);

	CAN_Begin(CAN_MODE_NORMAL);
	CAN_SetAllFilters();
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
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
  hi2c1.Init.Timing = 0x2000090E;
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
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

	  /*Configure GPIO pins : PC1 PC3 PC4 */
	  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_3|GPIO_PIN_4;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PB12 PB13 PB14 */
	  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	  /*Configure GPIO pin : PC7 */
	  GPIO_InitStruct.Pin = GPIO_PIN_7;
	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pins : PA9 PA10 */
	  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	uint8_t i = 0;
	for (i = 0 ; i < 32 ; i++){
		if ( (Error_Status >> i) & 0x01 ){
			PrintError(i);
		}
	}

	while(1);
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
