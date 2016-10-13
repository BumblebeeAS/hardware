#include <Arduino.h>
#include <Brushed.h>
#include <Math.h>

Brushed::Brushed(volatile uint8_t directions_Register_a,volatile uint8_t directions_Register_b, volatile uint8_t IO_Register, int SR, int RS, int PWM, int FF1, int FF2, int PWM_pin)
{	

	Channel = IO_Register;
	Channel_Reset = RS;
	PWM_Pin = PWM_pin;
	//Setting directions
	//SR, Reset, PWM pins are output
	directions_Register_a |= (SR | RS | PWM);
	directions_Register_b &= ~(FF1 | FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	IO_Register |= (RS | PWM);
	IO_Register &= ~SR;

	//Set PWM Frequency
	TCCR0B = (TCCR0B & 0b11111000) | 0x01;
  	TCCR1B = (TCCR1B & 0b11111000) | 0x01;
  	TCCR2B = (TCCR2B & 0b11111000) | 0x01;
}

Brushed::~Brushed()
{


}


void Brushed::init(void){
	reset();
}

void Brushed::reset(void)
{
	Channel &= ~(Channel_Reset);
	delay(100);
	Channel |= Channel_Reset;
	analogWrite(PWM_Pin, 128);
}



void Brushed::run(int speed)
{
	uint8_t pwm = map(speed, -3200, 3200, 20, 235);
	
	analogWrite(PWM_Pin, pwm);
	
}


void Brushed::stop(void)
{
	analogWrite(PWM_Pin, 128);
	
}

