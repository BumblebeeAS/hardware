# Description

This driver is a wrapper for the HAL_CAN driver.
This library is intended to be used for stm32f042 and stm32f072 only.
Some macro need to be redefined if this were to be used on other stm32 family.

# What you need to check before using this library

1. This library is intended to work with HSE=32Mhz at CAN speed 1Mpbs. If crystals or clocks with other frequencies were to be used, you need to go online and search "CAN Bittiming calculator", recalculated the required setting and then go to CAN_Init() function inside the CAN driver source code to redo the following settings marked by arrows.
```bash
 // initialise CAN
void CAN_Init(void)
{
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 2;		 <---
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ; <--- (This is usually set to 1)
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;     <---
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;	 <---
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if(HAL_CAN_Init(&hcan) != HAL_OK)
  {
	  Error_Status |= 1<< CAN_INIT_ERROR;
    Error_Handler();
  }
}
```

# Issue Log 3

1. CAN fails initialisation due to a variety of reasons. Some common fixes are described here.

1.1 CAN_TX and CAN_RX pins sometimes need to be configured to be PULLUP mode for it to work. Open stm32f0x_msp.c and find function called "void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)". This function initialises all the lower level peripherals that CAN controller requires. You should see this function looks some thing like the below one. Change GPIO PULL Mode to GPIO_PULLUP as shown by the line marked with arrow.
```bash
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hcan->Instance==CAN)
  {
    /* Peripheral clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    /* GPIO clock and pinsel config */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;		<---
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}
```

1.2 On STM32F4 series, short CAN_TX and CAN_RX pins with a jumper when using loopback mode. Even when the datasheet says that they are internally shorted in loopback mode, they clearly weren't when we tested it.

1.3 In normal CAN mode, it needs at least 1 more node on the bus to acknowledge it during initialisation. Connect the device under test to another reception node and test again.

1.4 120 ohm resistors at both ends of the bus are NOT optional. They are like pullup resistor that pull the bus lines together to recessive state.

1.5 All error flags related to CAN controller are reported at 
	
```bash
hcan.Instance->ESR;
```
Do check this register and stm32f0 user manual to see what the error message is.

2. Interrupt mode are tested but it runs into race condition frequently hence are not implemented. If you want to use it:
	1. Call HAL_CAN_ActivateNotification() function to initialise which interrupt to enable.
	2. Implement CEC_CAN_IRQHandler(void) and include HAL_CAN_IRQHandler(&hcan) inside the function. The HAL_CAN_IRQHandler will redirect the interrupt to the respective callback functions.
	3. Implement the respective callback functions described in the HAL_CAN driver.
	4. Enable NVIC by calling HAL_NVIC_EnableIRQ(30);
 




## CAN bandwidth:
Test Result:
   		Transmitter sending at 1ms interval (8 bytes msg) and receiver received 10000 msgs in 20036ms
		=> 500 msg/s, or 2ms interval

Theoratical Result:
		using CAN bandwith calculator from : http://www.esacademy.com/en/library/calculators/can-best-and-worst-case-calculator.html
		worst case bandwith will be 576kbps with one frame containing 111 bits (8byte msg, std id)
		Hence in the worst case , number of CAN msgs per second is
		=> 576k/111 = 5k msg/s, or 0.2ms interval


## UART bandwith:

Test Result:
		Transmitter(Python serial) sending at 5ms interval with 45 bytes (noise + data)
		=> 692 bytes/s, or 1.45ms interval


## Issue log 2:
In static void UART_RxISR_8BIT(UART_HandleTypeDef *huart), RXNE is cleared via
uhdata = (uint16_t) READ_REG(huart->Instance->RDR);

Small modifications  are made to static void UART_RxISR_8BIT(UART_HandleTypeDef *huart) in HAL_Uart.c
the source code now detects Start and stop bytes on the run

In Uart interrupt handler, receive callback of a single byte is called sneakily by huart->RxISR(huart). RxISR is a function pointer defined in the HAL_Uart_Receive_IT.
It in turn calls UART_RxISR_8BIT that handles putting byte into buffer( and clear RXNE)

Uart overrun shoud disabled / handled if necessary (not implemented yet)

Overrun interrupt must be enabled first to allow clearing

## Issue log 1:

Problem initialising GPIO PA0 PA4

Uart printf speed can be as slow as 2ms

