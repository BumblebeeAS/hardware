#ifndef _ACTUATOR_H
#define _ACTUATOR_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "MCP23S17.h"

#define CS_IO		5
#define MCP_ADDR	0x00

#define ROTARY_DEFAULT		150
#define LINEAR_DEFAULT		50
#define SHOOTER_DEFAULT		10

enum ActuationType : uint8_t {
	ROTATE_DOWN = 0x00,
	ROTATE_UP,
	LINEAR
};

struct _Actuator {
	uint8_t pin;				//corresponding pin at the IO expander
	uint16_t time;				//corresponding actuation time
	uint16_t currTime; 			//time which stores the current time value
	bool requestToDisconnect;	//flag to determine if the actuator needs to stop actuating
};

class Actuator
{
 private:
	 MCP23S17 mcp23s17 = { MCP_ADDR, CS_IO };
	 _Actuator rotateUp, rotateDown, linear, shooter[4];

 public:

	 Actuator(uint8_t rotary_deploy, uint8_t rotary_retrieve, uint8_t pin_linear, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t pin_s4);
	 Actuator(void);
	 
	 void init(void);

	 //linear and rotary actuator needs to be specifically turn off
	 void actuateAcoustic(ActuationType actuationType);
	 void stopActuation(ActuationType actuationType);
	 
	 //shooter will be automatically turn off using timer 1
	 void actuateShooter(uint8_t num);

	 //call back function to switch off the actuation
	 void actuatorCallBack(void);
};

#endif

