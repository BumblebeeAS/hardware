#include "ADS1115.h"

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void ADS_WriteRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value)
{
	ADS_dataW[0] = (uint8_t)reg;
	ADS_dataW[1] = (uint8_t)(value >> 8);
	ADS_dataW[2] = (uint8_t)(value & 0xFF);
	HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, ADS_dataW, 3, 10);
}
/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
uint16_t ADS_ReadRegister(uint8_t i2cAddress, uint8_t reg)
{
	ADS_dataW[0] = ADS1115_REG_POINTER_CONVERT;
	HAL_I2C_Master_Transmit(&hi2c1, i2cAddress, ADS_dataW, 1, 10);
	HAL_I2C_Master_Receive(&hi2c1, i2cAddress, &ADS_dataR[0], 2, 10);
	
	return ( (ADS_dataR[0] << 8) | ADS_dataR[1]);
}

/**************************************************************************/
/*!
    @brief  Setups the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
void ADS_Init() {
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

/**************************************************************************/
/*!
@brief  Setups the config register for continuous conversion mode and the 
channel for continuous conversion
*/
/**************************************************************************/

uint16_t ADS_SetContinuousConv(uint8_t channel)
{
	if (channel > 3) {
		return 0;
	}
	
	// Start with default values
	perm_config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
				  ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
				  ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
			   	  ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
				  ADS1115_REG_CONFIG_DR_3300SPS;    // 1600 samples per second (default)

	// Set single-ended input channel
	switch (channel) {
	case (0) :
		perm_config |= ADS1115_REG_CONFIG_MUX_SINGLE_0; break;
	case (1) :
		perm_config |= ADS1115_REG_CONFIG_MUX_SINGLE_1; break;
	case (2) :
		perm_config |= ADS1115_REG_CONFIG_MUX_SINGLE_2; break;
	case (3) :
		perm_config |= ADS1115_REG_CONFIG_MUX_SINGLE_3; break;
	}

	// Continuous mode, clear bit
	perm_config &= ADS1115_REG_CONFIG_OS_CONTINUOUS;
	
	// Set PGA/voltage range
	// +/- 6.144V range (limited to VDD +0.3V max!)
	perm_config |= ADS1115_REG_CONFIG_PGA_6_144V;
	
	// Write config register to the ADC
	ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, perm_config);
	return perm_config;
}

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t ADS_ReadADC_SingleEnded(uint8_t channel)
{
  if (channel > 3) {
    return 0;
  }
  
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1115_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set single-ended input channel
  switch (channel) {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0; break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1; break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2; break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3; break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
//  HAL_Delay(ADS1115_CONVERSIONDELAY);
  for (int i = 0; i<64; i++) {

  }

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1115
  return ADS_ReadRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> ADS1115_BITSHIFT;
}

/**************************************************************************/
/*!
@brief  Gets a continuous ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t ADS_ReadADC_Continuous()
{
	// Read the conversion results
	// Shift 12-bit results right 4 bits for the ADS1115
	return ADS_ReadRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> ADS1115_BITSHIFT;
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t ADS_ReadADC_Differential_0_1()
{
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1115_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  HAL_Delay(ADS1115_CONVERSIONDELAY);

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1115
  return (int16_t)(ADS_ReadRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> ADS1115_BITSHIFT);
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t ADS_ReadADC_Differential_2_3()
{
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1115_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set channels
  config |= ADS1115_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1115_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  HAL_Delay(ADS1115_CONVERSIONDELAY);

  // Shift 12-bit results right 4 bits for the ADS1115
  return (int16_t)(ADS_ReadRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> ADS1115_BITSHIFT);
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void ADS_StartComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
  // Start with default values
  uint16_t config = ADS1115_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1115_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1115_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1115_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1115_REG_CONFIG_PGA_6_144V   | // +/- 6.144V range (limited to VDD +0.3V max!)
                    ADS1115_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set single-ended input channel
  switch (channel) {
    case (0):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_0; break;
    case (1):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_1; break;
    case (2):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_2; break;
    case (3):
      config |= ADS1115_REG_CONFIG_MUX_SINGLE_3; break;
  }

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1115
  ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_HITHRESH, threshold << ADS1115_BITSHIFT);

  // Write config register to the ADC
  ADS_WriteRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t ADS_GetLastConversionResults()
{

  // Wait for the conversion to complete
  HAL_Delay(ADS1115_CONVERSIONDELAY);

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1115
  return (int16_t)(ADS_ReadRegister(ADS1115_ADDRESS, ADS1115_REG_POINTER_CONVERT) >> ADS1115_BITSHIFT);
}
