#ifndef _SENSOR_H
#define _SENSOR_H


#include "stm32f0xx_hal.h"			//systick, delay,...
#include "stm32f0xx_hal_i2c.h"		//can library
#include "stm32f0xx_hal_cortex.h"	//nvic
#include "stdio.h"
#include "stm32f0xx_hal.h"
#include "screen.h"

#define lsm6_addrt 0xD7
#define lsm6_addrr 0xD6
#define lis3_addrt 0x3D
#define lis3_addrr 0x3C
#define hih_addrr 0x4D
#define hih_addrt 0x4D
#define pressure_addrr 0x50
#define pressure_addrt 0x51

//external pressure sensor
#define MS5387_addrt 0xED
#define MS5387_addrr 0xEC
#define MS5387_RESET              0x1E
#define MS5387_ADC_READ           0x00
#define MS5387_PROM_READ          0xA0
#define MS5387_CONVERT_D1_8192    0x4A
#define MS5387_CONVERT_D2_8192    0x5A


I2C_HandleTypeDef hi2c1;


void init_LSM6(void);

void init_LIS3(void);

void LSM6_read(void);


void LIS3_read(void);

void HIH_init(void);

void HIH_read(void);

void IntPressure_read(void);

void set_led(uint16_t color);

void reset_led(void);

void ExtPress_read(void);

void ExtPress_init(void);

void ExtPress_read(void);

void Ext_Pressure_Cal(void);

void sensor_update();

#endif
