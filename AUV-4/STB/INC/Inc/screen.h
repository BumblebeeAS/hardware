#ifndef _SCREEN_H
#define _SCREEN_H


#ifdef __cplusplus
 extern "C" {
#endif


#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_spi.h"		//can library
#include "stm32f0xx_hal_cortex.h"	//nvic
#include "stdio.h"					//printf
#include "stm32f0xx_hal.h"
#include "sensor.h"

#define	BLACK            0x0000
#define	BLUE             0x001F
#define	RED              0xF800
#define	GREEN            0x07E0
#define CYAN             0x07FF
#define MAGENTA          0xF81F
#define YELLOW           0xFFE0
#define WHITE            0xFFFF

#define INT_STAT_COUNT 19
#define EXT_PRESS 0
#define INT_PRESS 1
#define PMB1_PRESS 2
#define PMB2_PRESS 3
#define PMB1_TEMP 4
#define PMB2_TEMP 5
#define CPU_TEMP 6
#define HUMIDITY 7
#define ST_TEMP 8
#define DNA_PRESS 9
#define IMU_G_X 10
#define IMU_G_Y 11
#define IMU_G_Z 12
#define IMU_A_X 13
#define IMU_A_Y 14
#define IMU_A_Z 15
#define IMU_M_X 16
#define IMU_M_Y 17
#define IMU_M_Z 18

#define POWER_STAT_COUNT 6
#define BATT1_CAPACITY 0
#define BATT2_CAPACITY 1
#define BATT1_CURRENT 2
#define BATT2_CURRENT 3
#define BATT1_VOLTAGE 4
#define BATT2_VOLTAGE 5

#define HB_COUNT 7
#define XAVIER 0
#define SBC_CAN 1
#define PCB 2
#define Thruster 3
#define Manipulator 4
#define PMB1 5
#define PMB2 6

#define STB_UP_COUNT 8
#define Red 0
#define Green 1
#define Blue 2
#define SCREEN 3
#define SENSOR 4
#define MAG 5
#define ACC 6
#define GYRO 7

#define SCREEN_LOOP 1000
#define HB_TIMEOUT 3000
#define HEARTBEAT_LOOP 500
#define STAT_TIMEOUT 2000



 SPI_HandleTypeDef hspi1;

 uint8_t spiTxBuf[2];
 uint8_t spiRxBuf[2];

 volatile uint16_t powerStats[POWER_STAT_COUNT];
 volatile  uint32_t boardHB[HB_COUNT];
 volatile uint32_t boardHB_timeout[HB_COUNT];
 volatile uint16_t internalStats[INT_STAT_COUNT];
 volatile uint16_t stbUP[STB_UP_COUNT];



 void writeCommand(uint8_t d);

  void writeData(uint8_t d);

  void writeReg(uint8_t reg, uint8_t val);


  uint8_t readData(void);
  uint8_t readReg(uint8_t reg);

  uint8_t readStatus(void);


  void PLLint(void);

  void initialize(void);

  void begin(void);

  void displayOn(void);

  void GPIOX(void);

  void PWM1Config(void);

  void PWM1out(void);

  uint8_t waitPoll(uint8_t reg,uint8_t waitflag);

  void rectHelper(int16_t x,int16_t y, int16_t w, int16_t h, uint16_t color);

  void fillScreen(uint16_t color);

  void textMode(void);

  void textSetCursor(uint16_t x, uint16_t y);

  void textTransparent(uint16_t foreColor);

  uint8_t qstrlen(const char *var);

  char *qstrcat(char *dest, const char *src);

  void textWrite(const char* buffer, uint16_t len);

  void textColor(uint16_t foreColor, uint16_t bgColor);

 void textenlarge(uint8_t scale);

 void set_cursor(uint32_t x,uint32_t y);

 void write_string(const char* var);

 void increment_row(void);

 void screen_default(void);

 void write_state(uint32_t var);

 void write_value_int(uint32_t var);

 void write_value_dp(uint32_t var,uint32_t dp);

 void write_value_woinc(uint32_t var);

 void update_screen(void);

 void update(void);

 void default_values(void);

 void update_internalstats(void);

 void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

 void check_hb(void);

#endif
