#include <Wire.h>
#include <Adafruit_ADS1015.h>

Adafruit_ADS1115 ads1015(72);

void setup(void)
{
	Serial.begin(9600);
	Serial.println("Hello!");

	Serial.println("Getting single-ended readings from AIN0..3");
	Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV)");
	ads1015.begin();
	ads1015.set_continuous_conv(0);
}

void loop(void)
{
	int16_t adc0, adc1, adc2, adc3;

	adc0 = ads1015.readADC_Continuous();
	Serial.print("AIN0: "); Serial.println(adc0);

	Serial.println(" ");

	delay(1000);
}