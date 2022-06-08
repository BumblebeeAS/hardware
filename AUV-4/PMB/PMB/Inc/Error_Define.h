#ifndef __ERROR_DEFINE_H
#define __ERROR_DEFINE_H

#include "main.h"

#define SYSCLOCK_INIT_ERROR 	1	//Check Crystal / oscillator
									//and all relevant configurations in SystemClock_Config()
#define SPI_INIT_ERROR 			2
#define TIM17_INIT_ERROR 		3
#define UART1_INIT_ERROR		4
#define UART2_INIT_ERROR		5
#define	CAN_FILTER_INIT_ERROR	6
#define	CAN_INIT_ERROR			7	//Check if CAN_RX & CAN_TX pins are in pull-up mode
									//Check CAN_MODE
#define	CAN_START_ERROR			8	//CAN Node cannot join network. Check bit timing
#define	CAN_SENDMSG_ERROR		9	//Check CAN Error status register.
									//Usually due to no acknowledgment from reception node or no 120 terminating resistor
#define	CAN_SETFILTER_ERROR		10

// 11 - 17 are directly copied from user manual
#define CAN_STUFF_ERROR			11
#define	CAN_FORM_ERROR			12
#define	CAN_ACKNOWLEDGMENT_ERROR	13
#define	CAN_BIT_RECESSIVE_ERROR	14
#define	CAN_BIT_DOMINANT_ERROR	15
#define	CAN_CRC_ERROR			16
#define	CAN_SOFTWARE_ERROR		17


void PrintError (uint8_t err_code);





#endif
