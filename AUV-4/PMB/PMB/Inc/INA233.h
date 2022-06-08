/*
 * I2C interface for INA233 Current and Power Monitor IC
 * Library written by Zhi Jie, 2017
 */

#ifndef INA233_H
#define INA233_H

#define ARRAY_SIZE 10

#define INA233_ADDRESS (0x45 << 1)
#define I2C_CMD_CALIB 0xD4
#define I2C_CMD_CONFIG 0xD5
#define I2C_CMD_VOLTAGE 0x88
#define I2C_CMD_CURRENT 0x89
#define I2C_CMD_SHUNTVOLT 0xD1
#define I2C_CMD_CLEAREIN 0xD6
#define I2C_CMD_READEIN 0x86

#define MAX_CURRENT 80
#define SHUNT_RES 1 //mOhm

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
//#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"
#include "stm32f0xx_hal_cortex.h"

uint8_t INA_dataW[10];
uint8_t INA_dataR[10];

extern I2C_HandleTypeDef hi2c1;

uint16_t _maxCurrent;
uint8_t _shuntValue;
uint8_t _addr;
float _currLsb;

void INA233(uint16_t maxCurrent, uint8_t shuntValue, uint8_t addr);
void INA_Init();
void INA_Calibrate();
void INA_ClrBuffer();
void INA_WriteRegister(uint8_t addr, uint8_t cmd, uint8_t numByteToSend);
void INA_ReadRegister(uint8_t addr, uint8_t cmd, uint8_t numByteToSend);
uint16_t INA_ReadVoltage();
int16_t INA_ReadShuntVoltage();
int16_t INA_ReadCurrent();
float INA_FloatCurrent(int16_t bCurrent);
void INA_ClearEnergyAcc() ;
uint32_t INA_ReadEnergy();

#endif
