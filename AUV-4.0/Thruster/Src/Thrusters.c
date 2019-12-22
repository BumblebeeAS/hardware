/*
 * Thrusters.c
 *
 *  Created on: Dec 18, 2019
 *      Author: Ben Chong
 */

#include "Thrusters.h"

#define RATE_LIN 50

/*** ThrusterInit
 * Initiates connection with ESC by sending 1500us duty cycle PWM for 2 seconds
 */
void ThrusterInit(Thruster *self, int id, int esc_pin, TIM_HandleTypeDef * htim, uint32_t channel){
	self->id = id;
	self->esc_pin = esc_pin;
	self->htim = htim;
	self->channel = channel;
	self->current_pwm = ESC_SYNC;
	__HAL_TIM_SET_COMPARE(htim, channel, self->current_pwm/2); // 750 set pulse width to 1.5ms
	HAL_Delay(2000);
}

/*** Increment Linear
 * If PWM difference is more than 50 from previous, limit it at 50.
 */
int increment_linear(int throttle_old, int throttle_new)
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

void increment(Thruster *self, int throttle_old, int throttle_new){
	int next_throttle;
	if (throttle_new != throttle_old)
	{
		next_throttle = increment_linear(throttle_old, throttle_new) / 2;
		self->current_pwm = next_throttle * 2;
		__HAL_TIM_SET_COMPARE(self->htim, self->channel, next_throttle);
	}
}

void mov(Thruster *self, int input_speed){
	int throttle, throttle_old;

	//Convert tick period into actual PWM period between 1ms and 2ms
	throttle_old = __HAL_TIM_GET_COMPARE(self->htim, self->channel) * 2;
	throttle = mapSpeed(input_speed);

	increment(self, throttle_old, throttle);
}

int mapSpeed(int speed){
	int throttle;
	if (speed > 0) throttle = thrusterForward(speed);
	if (speed < 0) throttle = thrusterReverse(speed);
	if (speed == 0) throttle = THROTTLE_STOP;
	return throttle;
}

int map(int input, int in_min, int in_max, int out_min, int out_max){
	double output;
	double slope = 1.0 * ((double) out_max - out_min) / (in_max - in_min);
	output = out_min + slope * (input - in_min);
	return output;
}

int thrusterForward(int input){
	int throttle = map(input, 1, 3200, 1450, 1000);
	return throttle;
}

int thrusterReverse(int input){
	int throttle = map(input, -3200, -1, 2000, 1550);
	return throttle;
}



