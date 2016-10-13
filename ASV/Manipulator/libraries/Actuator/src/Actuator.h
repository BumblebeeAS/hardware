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
#define SHOOTER_DEFAULT		500

enum ActuationStatusType : uint8_t {
	ROTATED = 0x00,
	ROTATE_EXTENDED,
	REST
};

enum ActuationType : uint8_t {
	ROTARY = 0x00,
	LINEAR,
	SHOOT
};

struct _Actuator {
	uint8_t pin;	//corresponding pin at the IO expander
	uint16_t time;	//corresponding actuation time
	uint16_t currTime; //time which the counter starts
	bool requestToDisconnect;	//boolean to determine the actuation should stop
};

class Actuator
{
 private:
	 MCP23S17 mcp23s17 = { MCP_ADDR, CS_IO };
	 ActuationStatusType actuationStatus;
	 _Actuator rotary_deploy, rotary_retrieve, linear, shooter[4];

 public:

	 Actuator(uint8_t rotary_deploy, uint8_t rotary_retrieve, uint8_t pin_linear, uint8_t pin_s1, uint8_t pin_s2, uint8_t pin_s3, uint8_t pin_s4);
	 Actuator(void);
	 
	 void init(void);

	 void deployAcoustic(ActuationType actuationType);
	 void retrieveAcoutic(ActuationType actuationType);
	 
	 void actuateShooter(uint8_t num);

	 //call back function to switch off the actuation
	 void actuatorCallBack(void);
};

extern Actuator actuator;

#endif

