#include <Arduino.h>
#include <Brushed.h>
#include <Math.h>

Brushed::Brushed()
{

}

void Brushed::begin(){
	//Setting PWM frequency
	TCCR0B = (TCCR0B & 0b11111000) | 0x01;
	TCCR1B = (TCCR1B & 0b11111000) | 0x01;
	TCCR2B = (TCCR2B & 0b11111000) | 0x01;

	//For Channel 3------------------------
	//Setting directions
	//SR, Reset, PWM pins are output
	//FF1 and FF2 pins are input
	DDRA |= (CHN_3_SR | CHN_3_RESET | CHN_3_PWM);
	DDRA &= ~(CHN_3_FF1 | CHN_3_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTA |= (CHN_3_RESET | CHN_3_PWM);
	PORTA &= ~CHN_3_SR;
	//For Channel 3-------------------------

	//For Channel 4-------------------------
	//Setting directions
	//SR, Reset, PWM pins are output
	//FF1 and FF2 pins are input
	DDRD |= (CHN_4_SR | CHN_4_RESET | CHN_4_PWM);
	DDRL &= ~(CHN_4_FF1 | CHN_4_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTD |= (CHN_4_RESET | CHN_4_PWM);
	PORTD &= ~(CHN_4_SR);
	//For Channel 4-------------------------

	//For Channel 7-------------------------
	//Setting directions
	DDRC |= (CHN_7_SR | CHN_7_RESET | CHN_7_PWM);
	DDRC &= ~(CHN_7_FF1 | CHN_7_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTC |= (CHN_7_RESET | CHN_7_PWM);
	PORTC &= ~(CHN_7_SR);
	//For Channel 7-------------------------

	//For Channel 5------------------------
	//Setting directions
	DDRJ |= (CHN_5_SR | CHN_5_RESET | CHN_5_PWM);
	DDRJ &= ~(CHN_5_FF1 | CHN_5_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTJ |= (CHN_5_RESET | CHN_5_PWM);
	PORTJ &= ~(CHN_5_SR);
	//For Channel 5-------------------------

	//For Channel 6-------------------------
	//Setting directions
	DDRF |= (CHN_6_SR | CHN_6_RESET | CHN_6_PWM);
	DDRF &= ~(CHN_6_FF1 | CHN_6_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTF |= (CHN_6_RESET | CHN_6_PWM);
	PORTF &= ~(CHN_6_SR);
	//For Channel 6-------------------------

	//For Channel 8-------------------------
	//Setting directions
	DDRK |= (CHN_8_SR | CHN_8_RESET | CHN_8_PWM);
	DDRK &= ~(CHN_8_FF1 | CHN_8_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTK |= (CHN_8_RESET | CHN_8_PWM);
	PORTK &= ~(CHN_8_SR);
	//For Channel 8-------------------------

	resetAll();
}

void Brushed::reset(uint8_t channel)
{
	switch(channel)
	{
	case '3':
		reset3();
		break;
	case '4':
		reset4();
		break;
	case '7':
		reset7();
		break;
	case '5':
		reset5();
		break;
	case '6':
		reset6();
		break;
	case '8':
		reset8();
		break;
	}
}

void Brushed::resetAll(void)
{
		reset3();
		reset4();
		reset7();
		reset5();
		reset6();
		reset8();
}

void Brushed::run(uint8_t channel, int16_t speed)
{
	uint8_t pwm = map(speed, -3200, 3200, 20, 235);
	switch(channel)
	{
	case '3':
		analogWrite(PWM3, pwm);
		break;
	case '4':
		analogWrite(PWM4, pwm);
		break;
	case '7':
		analogWrite(PWM7, pwm);
		break;
	case '5':
		analogWrite(PWM5, pwm);
		break;
	case '6':
		analogWrite(PWM6, pwm);
		break;
	case '8':
		analogWrite(PWM8, pwm);
		break;
	}
}

void Brushed::runAll(int16_t speed)
{
	uint8_t pwm = map(speed, -3200, 3200, 20, 235);
	analogWrite(PWM3, pwm);
	analogWrite(PWM4, pwm);
	analogWrite(PWM7, pwm);
	analogWrite(PWM5, pwm);
	analogWrite(PWM6, pwm);
	analogWrite(PWM8, pwm);
}

void Brushed::stop(uint8_t channel)
{
	switch(channel)
	{
	case '3':
		analogWrite(PWM3, 128);
		break;
	case '4':
		analogWrite(PWM4, 128);
		break;
	case '7':
		analogWrite(PWM7, 128);
		break;
	case '5':
		analogWrite(PWM5, 128);
		break;
	case '6':
		analogWrite(PWM6, 128);
		break;
	case '8':
		analogWrite(PWM8, 128);
		break;
	}
}

void Brushed::stopAll(void)
{
	analogWrite(PWM3, 128);
	analogWrite(PWM4, 128);
	analogWrite(PWM7, 128);
	analogWrite(PWM5, 128);
	analogWrite(PWM6, 128);
	analogWrite(PWM8, 128);
}

void Brushed::reset3(void)
{
	PORTA &= ~(CHN_3_RESET);
	delay(100);
	PORTA |= CHN_3_RESET;
	analogWrite(PWM3, 128);
}

void Brushed::reset4(void)
{
	PORTD &= ~(CHN_4_RESET);
	delay(100);
	PORTD |= CHN_4_RESET;
	analogWrite(PWM4, 128);
}

void Brushed::reset7(void)
{
	PORTC &= ~(CHN_7_RESET);
	delay(100);
	PORTC |= CHN_7_RESET;
	analogWrite(PWM7, 128);
}

void Brushed::reset5(void)
{
	PORTJ &= ~(CHN_5_RESET);
	delay(100);
	PORTJ |= CHN_5_RESET;
	analogWrite(PWM5, 128);
}

void Brushed::reset6(void)
{
	PORTF &= ~(CHN_6_RESET);
	delay(100);
	PORTF |= CHN_6_RESET;
	analogWrite(PWM6, 128);
}

void Brushed::reset8(void)
{
	PORTK &= ~(CHN_8_RESET);
	delay(100);
	PORTK |= CHN_8_RESET;
	analogWrite(PWM8, 128);
}
