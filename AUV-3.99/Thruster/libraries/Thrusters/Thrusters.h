#ifndef Thrusters_h
#define Thrusters_h

#include "Arduino.h"
#include <Servo.h>

#define THROTTLE_STOP 1500

class Thrusters
{
public:
	Thrusters(int thruster_num, Servo servo, int pwm_pin, int front_max, int front_min, int back_min, int back_max);
	void init();
	void mov(int input_speed);

private:
	int id; // Thruster number on AUV
	Servo esc;
	int esc_pin;
	int forward_max, forward_min; // To calibrate range of thruster
	int reverse_max, reverse_min;
	
	void acc(int input1, int input2);
	int increment(int, int);
	int increment_exp(int throttle_old, int throttle_new);
	int increment_linear(int throttle_old, int throttle_new);

	int mapSpeed(int speed);
	int thrusterForward(int input);
	int thrusterReverse(int input);
	int thrusterStop(void);
};

#endif
