#include "Error_Define.h"


void PrintError	(uint8_t err_code)
{
	switch (err_code)
	{
	case 1:
		printf("====SYSCLOCK_INIT_ERROR Detected===\r\n");
		break;
	case 2:
		printf("====SPI_INIT_ERROR Detected===\r\n");
		break;
	case 3:
		printf("====TIM17_INIT_ERROR Detected===\r\n");
		break;
	case 4:
		printf("====UART1_INIT_ERROR Detected===\r\n");
		break;
	case 5:
		printf("====UART2_INIT_ERROR Detected===\r\n");
		break;
	case 6:
		printf("====CAN_FILTER_INIT_ERROR Detected===\r\n");
		break;
	case 7:
		printf("====CAN_INIT_ERROR Detected===\r\n");
		break;
	case 8:
		printf("====CAN_START_ERROR Detected===\r\n");
		break;
	case 9:
		printf("====CAN_SENDMSG_ERROR Detected===\r\n");
		break;
	case 10:
		printf("====CAN_SETFILTER_ERROR Detected===\r\n");
		break;
	case 11:
		printf("====CAN_STUFF_ERROR Detected===\r\n");
		break;
	case 12:
		printf("====CAN_FORM_ERROR Detected===\r\n");
		break;
	case 13:
		printf("====CAN_ACKNOWLEDGMENT_ERROR Detected===\r\n");
		break;
	case 14:
		printf("====CAN_BIT_RECESSIVE_ERROR Detected===\r\n");
		break;
	case 15:
		printf("====CAN_BIT_DOMINANT_ERROR Detected===\r\n");
		break;
	case 16:
		printf("====CAN_CRC_ERROR Detected===\r\n");
		break;
	case 17:
		printf("====CAN_SOFTWARE_ERROR Detected===\r\n");
		break;
	default:
		printf("====UNKNOWN_ERROR Detected===\r\n");
		break;
	}

}
