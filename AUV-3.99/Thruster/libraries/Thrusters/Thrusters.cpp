#include <Arduino.h>
#include <Servo.h>
#include "Thrusters.h"
#include <Math.h>

#define RATE 0.05  /// MUST BE LESS THAN 1

Thrusters::Thrusters(int thruster_num, Servo servo, int pwm_pin, int front_max, int front_min, int back_min, int back_max)
{
	id = thruster_num;
	esc = servo;
	esc_pin = pwm_pin;
	forward_max = front_max;
	forward_min = front_min;
	reverse_max = back_max;
	reverse_min = back_min;
}

void Thrusters::init()
{
	esc.attach(esc_pin);
}

// Increase throttle by RATE
int Thrusters::increment(int throttle_old, int throttle_new) {
	int next_throttle;
	if (throttle_new > throttle_old) {
		// Forward
		next_throttle = throttle_old + ceil(RATE * (throttle_new - throttle_old));
		esc.writeMicroseconds(next_throttle);
	}
	else if (throttle_old > throttle_new) {
		// Reverse
		next_throttle = throttle_old - ceil(RATE * (throttle_old - throttle_new));
		esc.writeMicroseconds(next_throttle);
	}
	// Don't do anything if both throttles are same

	Serial.print("ESC");
	Serial.print(id);
	Serial.print(" Throttle: ");
	Serial.println(next_throttle);
	return next_throttle;
}

// Increase throttle gradually by recursively increasing throttle
void Thrusters::acc(int throttleOld1, int throttle_new)
{
	if (throttleOld1 != throttle_new)
		acc(increment(throttleOld1, throttle_new), throttle_new);
	return;
}

void Thrusters::mov(int input_speed)
{
	int throttle, throttleOld;

	throttleOld = esc.readMicroseconds();
	throttle = mapSpeed(input_speed);

	acc(throttleOld, throttle);
	return;
}

int Thrusters::mapSpeed(int speed)
{
	int throttle;
	if (speed > 0) { throttle = thrusterForward(speed); }
	if (speed < 0) { throttle = thrusterReverse(speed); }
	if (speed == 0) { throttle = thrusterStop(); }
	return throttle;
}

int Thrusters::thrusterForward(int input)
{
	int throttle = map(input, 1, 3200, forward_min, forward_max);
	return throttle;
}

int Thrusters::thrusterReverse(int input)
{
	int throttle = map(input, -3200, -1, reverse_max, reverse_min);
	return throttle;
}

int Thrusters::thrusterStop(void)
{
	return 1500;
}
