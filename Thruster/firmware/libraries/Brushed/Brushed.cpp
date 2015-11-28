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

	//For Channel 1------------------------
	//Setting directions
	//SR, Reset, PWM pins are output
	//FF1 and FF2 pins are input
	DDRA |= (CHN_1_SR | CHN_1_RESET | CHN_1_PWM);
	DDRA &= ~(CHN_1_FF1 | CHN_1_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTA |= (CHN_1_RESET | CHN_1_PWM);
	PORTA &= ~CHN_1_SR;
	//For Channel 1-------------------------

	//For Channel 2-------------------------
	//Setting directions
	//SR, Reset, PWM pins are output
	//FF1 and FF2 pins are input
	DDRD |= (CHN_2_SR | CHN_2_RESET | CHN_2_PWM);
	DDRL &= ~(CHN_2_FF1 | CHN_2_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTD |= (CHN_2_RESET | CHN_2_PWM);
	PORTD &= ~(CHN_2_SR);
	//For Channel 2-------------------------

	//For Channel 3-------------------------
	//Setting directions
	DDRC |= (CHN_3_SR | CHN_3_RESET | CHN_3_PWM);
	DDRC &= ~(CHN_3_FF1 | CHN_3_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTC |= (CHN_3_RESET | CHN_3_PWM);
	PORTC &= ~(CHN_3_SR);
	//For Channel 3-------------------------

	//For Channel 4------------------------
	//Setting directions
	DDRJ |= (CHN_4_SR | CHN_4_RESET | CHN_4_PWM);
	DDRJ &= ~(CHN_4_FF1 | CHN_4_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTJ |= (CHN_4_RESET | CHN_4_PWM);
	PORTJ &= ~(CHN_4_SR);
	//For Channel 4-------------------------

	//For Channel 5-------------------------
	//Setting directions
	DDRF |= (CHN_5_SR | CHN_5_RESET | CHN_5_PWM);
	DDRF &= ~(CHN_5_FF1 | CHN_5_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTF |= (CHN_5_RESET | CHN_5_PWM);
	PORTF &= ~(CHN_5_SR);
	//For Channel 5-------------------------

	//For Channel 6-------------------------
	//Setting directions
	DDRK |= (CHN_6_SR | CHN_6_RESET | CHN_6_PWM);
	DDRK &= ~(CHN_6_FF1 | CHN_6_FF2);

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTK |= (CHN_6_RESET | CHN_6_PWM);
	PORTK &= ~(CHN_6_SR);
	//For Channel 6-------------------------

	resetAll();
}

void Brushed::reset(uint8_t channel)
{
	switch(channel)
	{
	case '1':
		reset1();
	case '2':
		reset2();
	case '3':
		reset3();
	case '4':
		reset4();
	case '5':
		reset5();
	case '6':
		reset6();
	}
}

void Brushed::resetAll(void)
{
		reset1();
		reset2();
		reset3();
		reset4();
		reset5();
		reset6();
}

void Brushed::run(uint8_t channel, uint8_t speed)
{
	uint8_t pwm = map(speed, -3200, 3200, 20, 230);
	switch(channel)
	{
	case '1':
		analogWrite(PWM1, pwm);
	case '2':
		analogWrite(PWM2, pwm);
	case '3':
		analogWrite(PWM3, pwm);
	case '4':
		analogWrite(PWM4, pwm);
	case '5':
		analogWrite(PWM5, pwm);
	case '6':
		analogWrite(PWM6, pwm);
	}
}

void Brushed::runAll(uint8_t speed)
{
	uint8_t pwm = map(speed, -3200, 3200, 20, 230);
	analogWrite(PWM1, pwm);
	analogWrite(PWM2, pwm);
	analogWrite(PWM3, pwm);
	analogWrite(PWM4, pwm);
	analogWrite(PWM5, pwm);
	analogWrite(PWM6, pwm);
}

void Brushed::stop(uint8_t channel)
{
	switch(channel)
	{
	case '1':
		analogWrite(PWM1, 128);
	case '2':
		analogWrite(PWM2, 128);
	case '3':
		analogWrite(PWM3, 128);
	case '4':
		analogWrite(PWM4, 128);
	case '5':
		analogWrite(PWM5, 128);
	case '6':
		analogWrite(PWM6, 128);
	}
}

void Brushed::stopAll(void)
{
	analogWrite(PWM1, 128);
	analogWrite(PWM2, 128);
	analogWrite(PWM3, 128);
	analogWrite(PWM4, 128);
	analogWrite(PWM5, 128);
	analogWrite(PWM6, 128);
}

void Brushed::reset1(void)
{
	PORTA &= ~(CHN_1_RESET);
	delay(100);
	PORTA |= CHN_1_RESET;
	analogWrite(PWM1, 128);
}

void Brushed::reset2(void)
{
	PORTD &= ~(CHN_2_RESET);
	delay(100);
	PORTD |= CHN_2_RESET;
	analogWrite(PWM2, 128);
}

void Brushed::reset3(void)
{
	PORTC &= ~(CHN_3_RESET);
	delay(100);
	PORTC |= CHN_3_RESET;
	analogWrite(PWM3, 128);
}

void Brushed::reset4(void)
{
	PORTJ &= ~(CHN_4_RESET);
	delay(100);
	PORTJ |= CHN_4_RESET;
	analogWrite(PWM4, 128);
}

void Brushed::reset5(void)
{
	PORTF &= ~(CHN_5_RESET);
	delay(100);
	PORTF |= CHN_5_RESET;
	analogWrite(PWM5, 128);
}

void Brushed::reset6(void)
{
	PORTK &= ~(CHN_6_RESET);
	delay(100);
	PORTK |= CHN_6_RESET;
	analogWrite(PWM6, 128);
}
