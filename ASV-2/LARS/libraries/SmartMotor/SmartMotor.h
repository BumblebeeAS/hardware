#pragma once
#ifndef _SMART_MOTOR_
#define _SMART_MOTOR_

#include"Arduino.h"

#define MAX_PACKET_SIZE 40 //in bytes

#define RX_ENABLE 22
#define DX_ENABLE 24

#define BAUDRATE 9600

/*=======================================================
				PACKET DELIMITERS
	-------------------------------------------------------*/
	#define PACKET_END_RX '\r' /* marks end of every packet motor sends*/
	#define PACKET_END_TX ' ' /* marks end of every packet we send*/

/*=======================================================
				REQUEST TYPE
	-------------------------------------------------------*/
	#define REQUEST_POSITION 1
	#define REQUEST_GOAL 2
	#define REQUEST_VELOCITY 3
	#define REQUEST_ACCELERATION 4
	#define REQUEST_TRAJECTORY_FLAG 5
	// Add on other stuff you want to request

	typedef struct {
	// TODO: You go double check the datatypes 
	int32_t acceleration;
	int32_t velocity;
	int32_t position;
	int32_t goal;
	int16_t mode;
	
	} MotorDrive;
	
typedef struct {
	// Whatever flags, data, current, state blabla you want
	int32_t acceleration;
	int32_t velocity;
	int32_t position;
	int32_t goal;
	int32_t trajectory_flag;
	int16_t mode;
	
} MotorState;

class SmartMotor
{
private:
	MotorDrive _motorDrive;
	MotorState _motorState;

	int request_type;

	bool msgStart  = false;
	byte data[MAX_PACKET_SIZE];
	int len = 0; //length of packet body including header
	int32_t data_integer;

public:
	int startUpCount = 0;
	bool kill = false;

	SmartMotor();
	~SmartMotor();
	void init();
	void startup();

	void setPosition(int32_t value);
	void setPositionRelative(int32_t value);
	void setVelocity(int32_t value);
	void setAcceleration(int32_t value);
	void requestPosition();
	void requestGoal();
	void requestVelocity();
	void requestAcceleration();
	void requestTrajectoryFlag();
	int32_t getPosition();
	int32_t getGoal();
	int32_t getVelocity();
	int32_t getAcceleration();
	int8_t getTrajectoryFlag();
	void setRun(bool run_status);
	
	bool readMessage();
	void sendMessage(char msg[]);

	MotorState getMotorState();

private:
	void EncodeMessage();
	void decodeMessage();
};

#endif