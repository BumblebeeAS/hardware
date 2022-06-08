#include "LEDS.h"

LEDS::LEDS(int pin1, int pin2, int pin3)
{
	RedPin = pin1;
	GreenPin = pin2;
	BluePin = pin3;

	pinMode(RedPin, OUTPUT);
	pinMode(GreenPin, OUTPUT);
	pinMode(BluePin, OUTPUT);

	time = 0;
	blink_on = false;
}

void LEDS::setcolour(int pin1, int pin2, int pin3)
{
	analogWrite(RedPin, pin1);
	analogWrite(GreenPin, pin2);
	analogWrite(BluePin, pin3);
}

// a for first colour,	b for second colour,
// period in ms
void LEDS::blink(uint8_t a, uint8_t b, uint32_t period) {
	if (a > 10 || b > 10) {
		return;
	}
	if (millis() - time > period) {
		if (blink_on) {
			colour(a);
		}
		else {
			colour(b);
		}
		blink_on = !blink_on;
		time = millis();
	}
}


// 9 for off
void LEDS::colour(int colour)
{
	switch (colour)
	{
	case 0://Off
		setcolour(0, 0, 0);
		break;
	case 1://Red
		setcolour(255, 0, 0);
		break;
	case 2://Violet
		setcolour(238, 130, 238);
		break;
	case 3://Pink
		setcolour(255, 204, 255);
		break;
	case 4://Blue
		setcolour(0, 0, 255);
		break;
	case 5://Green
		setcolour(0, 255, 0);
		break;
	case 6://Cyan
		setcolour(0, 255, 255);
		break;
	case 7://Maroon
		setcolour(128, 0, 0);
		break;
	case 8://White
		setcolour(255, 255, 255);
		break;
	case 9://Yellow
		setcolour(255, 255, 0);
		break;
	default:
		setcolour(0, 0, 0);
		break;
	}
}