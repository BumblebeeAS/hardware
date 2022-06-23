// All credit to Carmine Noviello for this code
// https://github.com/cnoviello/mastering-stm32/blob/master/nucleo-f030R8/system/include/retarget/retarget.h

// code source:
// http://shawnhymel.com/1873/how-to-use-printf-on-stm32/

#ifndef _RETARGET_H__
#define _RETARGET_H__

#include "stm32f0xx_hal.h"
#include <sys/stat.h>

void RetargetInit(UART_HandleTypeDef *huart);	//call this function to initialise printf function
int _isatty(int fd);
int _write(int fd, char* ptr, int len);
int _close(int fd);
int _lseek(int fd, int ptr, int dir);
int _read(int fd, char* ptr, int len);
int _fstat(int fd, struct stat* st);

#endif //#ifndef _RETARGET_H__
