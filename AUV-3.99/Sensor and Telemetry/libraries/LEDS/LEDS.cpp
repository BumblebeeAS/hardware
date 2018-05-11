#include "LEDS.h"

LEDS::LEDS(int pin1,int pin2,int pin3)
{
	RedPin = pin1;
	GreenPin = pin2;
	BluePin = pin3;

	pinMode(RedPin, OUTPUT);
	pinMode(GreenPin, OUTPUT);
	pinMode(BluePin, OUTPUT);

	time = 0;
	blink_on = 0;
}

void LEDS::setcolour(int red, int green, int blue)
{
	if (red == 1) {
		digitalWrite(RedPin, HIGH);
	}
	else {
		digitalWrite(RedPin, LOW);
	}
	if (green == 1) {
		digitalWrite(GreenPin, HIGH);
	}
	else {
		digitalWrite(GreenPin, LOW);
	}
	if (blue == 1) {
		digitalWrite(BluePin, HIGH);
	}
	else {
		digitalWrite(BluePin, LOW);
	}
}

// a for first colour,	b for second colour,
// period in ms
void LEDS::blink(uint8_t a, uint8_t b, uint32_t period) {
	if (a > 9 || b > 9) {
		return;
	}
	if (millis() - time > period) {
		if (blink_on) {
			colour(a);
		}
		else {
			colour(b);
		}
		time = millis();
	}
}

// 9 for off
void LEDS::colour(int col)
{
	switch (col)
	{
	case 1://Red
		setcolour(1, 0, 0);
		break;
	case 2://Orange
		setcolour(1, 1, 0);
		break;
	case 3://Yellow
		setcolour(1, 1, 0);
		break;
	case 4://Green
		setcolour(0, 1, 0);
		break;
	case 5://Blue
		setcolour(0, 0, 1);
		break;
	case 6://indigo
		setcolour(1, 0, 1);
		break;
	case 7://violet
		setcolour(1, 0, 1);
		break;
	case 8://white
		setcolour(1, 1, 1);
		break;
	case 9://off
		setcolour(0, 0, 0);
		break;
	case 10://pink
		setcolour(1, 0, 1);
		break;
	default:
		setcolour(0, 0, 0);
		break;
	}
}

/*
void LEDS::colour(int col)
{
	switch (col)	// RGB
	{
	case 0:	//	Off
		setcolour(0, 0, 0);
		break;
	case 1:	//	Red
		setcolour(1, 0, 0);
		break;
	case 2:	//	Green
		setcolour(0, 1, 0);
		break;
	case 3:	//	Blue
		setcolour(0, 0, 1);
		break;
	case 4:	//	Cyan
		setcolour(0, 1, 1);
		break;
	case 5:	//	Magenta
		setcolour(1, 0, 1);
		break;
	case 6:	//	Yellow
		setcolour(1, 1, 0);
		break;
	case 7:	//	White
		setcolour(1, 1, 1);
		break;
	default:
		setcolour(0, 0, 0);
		break;
	}
}
*/