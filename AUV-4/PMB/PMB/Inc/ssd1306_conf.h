/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_i2c.h"
#include "stm32f0xx_hal_cortex.h"

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// Choose a microcontroller family
#define STM32F0
//#define STM32F1
//#define STM32F4
//#define STM32L0
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7

// Choose a bus
#define SSD1306_USE_I2C
//#define SSD1306_USE_SPI

// I2C Configuration
extern I2C_HandleTypeDef hi2c1;
#define SSD1306_I2C_PORT        hi2c1
#define SSD1306_I2C_ADDR        (0x78)

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Include only needed fonts
#define SSD1306_INCLUDE_FONT_6x8 1
#define SSD1306_INCLUDE_FONT_7x10 1
#define SSD1306_INCLUDE_FONT_11x18 1
#define SSD1306_INCLUDE_FONT_16x26 1

#endif /* __SSD1306_CONF_H__ */
