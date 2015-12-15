#include <Adafruit_ADS1015.h>
#include <Wire.h>

Adafruit_ADS1115 ADS75(75);
uint16_t ADC_result;

void setup()
{
  Serial.begin(9600);
  ADS75.begin();
}

void loop()
{
  
//  ADC_result = ADS75.readADC_SingleEnded(0);
//  if (ADC_result > 32000)
//    ADC_result = 0;
//  Serial.print("AIN1  ");
//  Serial.print(ADC_result);
//  
//  ADC_result = ADS75.readADC_SingleEnded(1);
//  if (ADC_result > 32000)
//    ADC_result = 0;
//  Serial.print("  AIN2  ");
//  Serial.print(ADC_result);
  
  ADC_result = ADS75.readADC_SingleEnded(2);
  if (ADC_result > 32000)
    ADC_result = 0;
  Serial.print("  AIN3  ");
  Serial.print(ADC_result);
  
  ADC_result = ADS75.readADC_SingleEnded(3);
  if (ADC_result > 32000)
    ADC_result = 0;
  Serial.print("  AIN4  ");
  Serial.println(ADC_result);
  
  delay(500);
}


