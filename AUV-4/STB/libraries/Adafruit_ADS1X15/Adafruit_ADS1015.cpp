/**************************************************************************/
/*!
    @file     Adafruit_ADS1015.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    Driver for the ADS1015/ADS1115 ADC

    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/???

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#include "Adafruit_ADS1015.h"

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static uint8_t i2cread(void) {
  #if ARDUINO >= 100
  return Wire.read();
  #else
  return Wire.receive();
  #endif
}

/**************************************************************************/
/*!
    @brief  Abstract away platform differences in Arduino wire library
*/
/**************************************************************************/
static void i2cwrite(uint8_t x) {
  #if ARDUINO >= 100
  Wire.write((uint8_t)x);
  #else
  Wire.send(x);
  #endif
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  Wire.beginTransmission(i2cAddress);
  i2cwrite((uint8_t)reg);
  i2cwrite((uint8_t)(value>>8));
  i2cwrite((uint8_t)(value & 0xFF));
  Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
static uint16_t readRegister(uint8_t i2cAddress, uint8_t reg) {
  Wire.beginTransmission(i2cAddress);
  i2cwrite(ADS1015_REG_POINTER_CONVERT);
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, (uint8_t)2);
  return ((i2cread() << 8) | i2cread());  
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties
*/
/**************************************************************************/
Adafruit_ADS1015::Adafruit_ADS1015(uint8_t i2cAddress) 
{
   m_i2cAddress = i2cAddress;
   m_conversionDelay = ADS1015_CONVERSIONDELAY;
   m_bitShift = 4;
}


/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
Adafruit_ADS1115::Adafruit_ADS1115(uint8_t i2cAddress)
{
   m_i2cAddress = i2cAddress;
   m_conversionDelay = ADS1115_CONVERSIONDELAY;
   m_bitShift = 0;
}

/**************************************************************************/
/*!
    @brief  Setups the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
void Adafruit_ADS1015::begin() {
  Wire.begin();
}

/**************************************************************************/
/*!
@brief  Setups the config register for continuous conversion mode and the 
channel for continuous conversion
*/
/**************************************************************************/

uint16_t Adafruit_ADS1015::set_continuous_conv(uint8_t channel) {
	if (channel > 3)
	{
		return 0;
	}

	// Start with default values
	perm_config = ADS1015_REG_CONFIG_CQUE_NONE | // Disable the comparator (default val)
		ADS1015_REG_CONFIG_CLAT_NONLAT | // Non-latching (default val)
		ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
		ADS1015_REG_CONFIG_CMODE_TRAD | // Traditional comparator (default val)
		ADS1015_REG_CONFIG_DR_3300SPS; // 1600 samples per second (default)


	// Set single-ended input channel
	switch (channel)
	{
	case (0) :
		perm_config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
		break;
	case (1) :
		perm_config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
		break;
	case (2) :
		perm_config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
		break;
	case (3) :
		perm_config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
		break;
	}

	//continuous mode, clear bit
	perm_config &= ADS1015_REG_CONFIG_OS_CONTINUOUS;
	// Set PGA/voltage range
	perm_config |= ADS1015_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)
	// Write config register to the ADC
	writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, perm_config);
	return perm_config;
}

/**************************************************************************/
/*!
    @brief  Gets a single-ended ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t Adafruit_ADS1015::readADC_SingleEnded(uint8_t channel) {
  if (channel > 3)
  {
    return 0;
  }
  
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1015_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set single-ended input channel
  switch (channel)
  {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(m_conversionDelay);

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  return readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;  
}

/**************************************************************************/
/*!
@brief  Gets a continuous ADC reading from the specified channel
*/
/**************************************************************************/
uint16_t Adafruit_ADS1015::readADC_Continuous() {
	// Read the conversion results
	// Shift 12-bit results right 4 bits for the ADS1015
	return readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN0) and N (AIN1) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::readADC_Differential_0_1() {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1015_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1;          // AIN0 = P, AIN1 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(m_conversionDelay);

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  return (int16_t)(readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift);  
}

/**************************************************************************/
/*! 
    @brief  Reads the conversion results, measuring the voltage
            difference between the P (AIN2) and N (AIN3) input.  Generates
            a signed value since the difference can be either
            positive or negative.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::readADC_Differential_2_3() {
  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
                    ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

  // Set PGA/voltage range
  config |= ADS1015_REG_CONFIG_PGA_6_144V;            // +/- 6.144V range (limited to VDD +0.3V max!)

  // Set channels
  config |= ADS1015_REG_CONFIG_MUX_DIFF_2_3;          // AIN2 = P, AIN3 = N

  // Set 'start single-conversion' bit
  config |= ADS1015_REG_CONFIG_OS_SINGLE;

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);

  // Wait for the conversion to complete
  delay(m_conversionDelay);

  // Shift 12-bit results right 4 bits for the ADS1015
  return (int16_t)(readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift);  
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void Adafruit_ADS1015::startComparator_SingleEnded(uint8_t channel, int16_t threshold)
{
  uint16_t value;

  // Start with default values
  uint16_t config = ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
                    ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching mode
                    ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
                    ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
                    ADS1015_REG_CONFIG_DR_1600SPS   | // 1600 samples per second (default)
                    ADS1015_REG_CONFIG_MODE_CONTIN  | // Continuous conversion mode
                    ADS1015_REG_CONFIG_PGA_6_144V   | // +/- 6.144V range (limited to VDD +0.3V max!)
                    ADS1015_REG_CONFIG_MODE_CONTIN;   // Continuous conversion mode

  // Set single-ended input channel
  switch (channel)
  {
    case (0):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_0;
      break;
    case (1):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_1;
      break;
    case (2):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_2;
      break;
    case (3):
      config |= ADS1015_REG_CONFIG_MUX_SINGLE_3;
      break;
  }

  // Set the high threshold register
  // Shift 12-bit results left 4 bits for the ADS1015
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_HITHRESH, threshold << m_bitShift);

  // Write config register to the ADC
  writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t Adafruit_ADS1015::getLastConversionResults()
{
  // Wait for the conversion to complete
  delay(m_conversionDelay);

  // Read the conversion results
  // Shift 12-bit results right 4 bits for the ADS1015
  return (int16_t)(readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift);
}

