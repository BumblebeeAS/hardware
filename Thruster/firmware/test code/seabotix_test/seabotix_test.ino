uint8_t pwm = 0;

void setup()
{
	initThrusters();
	resetThrusters();
	Serial.begin(9600);
}

void loop()
{
  pwm = 230;
    		analogWrite(13, pwm); //Channel 1
  		analogWrite(4, pwm); //Channel 2
		analogWrite(11, pwm); //Channel 3

		analogWrite(12, pwm); //Channel 4
		analogWrite(10, pwm); //Channel 5
		analogWrite(9, pwm); //Channel 6

		
        delay(500000);
		pwm = 20;
  		analogWrite(13, pwm); //Channel 1
  		analogWrite(4, pwm); //Channel 2
		analogWrite(11, pwm); //Channel 3

		analogWrite(12, pwm); //Channel 4
		analogWrite(10, pwm); //Channel 5
		analogWrite(9, pwm); //Channel 6


        delay(500000);
}

void initThrusters()
{
	/*Setting directions of the pins
	SR, Reset, PWM is OUTPUT
	FF1 and FF2 is INPUT
	Refer to pin mapping on trello*/

	//Setting PWM frequency
	TCCR0B = TCCR0B & 0b11111000 | 0x01;
	TCCR1B = TCCR1B & 0b11111000 | 0x01;
	TCCR2B = TCCR2B & 0b11111000 | 0x01;

	//For Channel 1------------------------
	//Setting directions
	DDRA |= 0B00011100;
	DDRA &= 0B11111100;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTA |= 0B00001100;
	PORTA &= 0B11101111;
	//For Channel 1-------------------------

	//For Channel 2-------------------------
	//Setting directions
	DDRD |= 0B00011100;
	DDRL &= 0B00111111;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTD |= 0B00001100;
	PORTD &= 0B11101111;
	//For Channel 2-------------------------

	//For Channel 3-------------------------
	//Setting directions
	DDRC |= 0B00000111;
	DDRC &= 0B11100111;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTC |= 0B00000110;
	PORTC &= 0B11111110;
	//For Channel 3-------------------------

	//For Channel 4------------------------
	//Setting directions
	DDRJ |= 0B00011100;
	DDRJ &= 0B11111100;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTJ |= 0B00001100;
	PORTJ &= 0B11101111;
	//For Channel 4-------------------------

	//For Channel 5-------------------------
	//Setting directions
	DDRF |= 0B00000111;
	DDRF &= 0B11100111;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTF |= 0B00000110;
	PORTF &= 0B11111110;
	//For Channel 5-------------------------

	//For Channel 6-------------------------
	//Setting directions
	DDRK |= 0B00000111;
	DDRK &= 0B11100111;

	//Setting Outputs
	//SR = LOW; Reset = HIGH; PWM = HIGH;
	PORTK |= 0B00000110;
	PORTK &= 0B11111110;
	//For Channel 6-------------------------
}

void resetThrusters()
{
	//For Channel 1
	PORTA &= 0B11111011;
	delay(100);
	PORTA |= 0B00000100;
	pinMode(13, OUTPUT);
	analogWrite(13, 128);

	//For Channel 2
	PORTD &= 0B11111011;
	delay(100);
	PORTD |= 0B00000100;
	pinMode(4, OUTPUT);
	analogWrite(4, 128);

	//For Channel 3
	PORTC &= 0B11111011;
	delay(100);
	PORTC |= 0B00000100;
	pinMode(11, OUTPUT);
	analogWrite(11, 128);

	//For Channel 4
	PORTJ &= 0B11111011;
	delay(100);
	PORTJ |= 0B00000100;
	pinMode(12, OUTPUT);
	analogWrite(12, 128);

	//For Channel 5
	PORTF &= 0B11111011;
	delay(100);
	PORTF |= 0B00000100;
	pinMode(10, OUTPUT);
	analogWrite(10, 128);

	//For Channel 6
	PORTK &= 0B11111011;
	delay(100);
	PORTK |= 0B00000100;
	pinMode(9, OUTPUT);
	analogWrite(9, 128);
}
