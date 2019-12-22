#ifndef Thrusters_h
#define Thrusters_h

#define ESC_SYNC 1500
#define THROTTLE_STOP 1500
#include "stm32f0xx_hal.h"

typedef struct Thruster{	
	int id; // Thruster number on AUV
	int esc_pin;
	int current_pwm;
	TIM_HandleTypeDef * htim;
	uint32_t channel; //EG TIM_CHANNEL_1
	
} Thruster; 

void ThrusterInit(Thruster *self, int id, int esc_pin, TIM_HandleTypeDef * htim, uint32_t channel);
void mov(Thruster *self, int input_speed);
int mapSpeed(int speed);
int thrusterForward(int input);
int thrusterReverse(int input);

#endif
