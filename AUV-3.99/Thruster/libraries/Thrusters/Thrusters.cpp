#include <Arduino.h>
#include <Servo.h>
#include "Thrusters.h"
#include <Math.h>

#define RATE_EXP 0.05  /// MUST BE LESS THAN 1
#define RATE_LIN 50

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
	esc.writeMicroseconds(THROTTLE_STOP);
}

// Increase throttle by RATE
int Thrusters::increment(int throttle_old, int throttle_new) {
	int next_throttle;
	// Don't do anything if both throttles are same
	if (throttle_new != throttle_old)
	{
		next_throttle = increment_linear(throttle_old, throttle_new);
		esc.writeMicroseconds(next_throttle);
	}
	/*
	Serial.print("ESC");
	Serial.print(id);
	Serial.print(" Throttle: old-");
	Serial.print(throttle_old);
	Serial.print(" next-");
	Serial.print(next_throttle);
	Serial.print(" final-");
	Serial.println(throttle_new);*/
	return next_throttle;
}

int Thrusters::increment_linear(int throttle_old, int throttle_new)
{
	int next_throttle;
	if (throttle_new > throttle_old) {
		// Forward
		next_throttle = throttle_old + ((throttle_new - throttle_old) > RATE_LIN ? RATE_LIN : (throttle_new - throttle_old));
	}
	else if (throttle_old > throttle_new) {
		// Reverse
		next_throttle = throttle_old - ((throttle_old - throttle_new) > RATE_LIN ? RATE_LIN : (throttle_old - throttle_new));
	}
	return next_throttle;
}

int Thrusters::increment_exp(int throttle_old, int throttle_new)
{
	int next_throttle;
	if (throttle_new > throttle_old) {
		// Forward
		next_throttle = throttle_old + ceil(RATE_EXP * (throttle_new - throttle_old));
	}
	else if (throttle_old > throttle_new) {
		// Reverse
		next_throttle = throttle_old - ceil(RATE_EXP * (throttle_old - throttle_new));
	}
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

	increment(throttleOld, throttle);
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
	return THROTTLE_STOP;
}
