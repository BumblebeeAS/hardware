# AUV 4.0 CAN

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
