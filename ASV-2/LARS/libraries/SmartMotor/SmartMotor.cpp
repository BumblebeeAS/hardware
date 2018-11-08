#include "SmartMotor.h"
#include <Arduino.h>
#include <stdio.h>

SmartMotor::SmartMotor()
{
}

SmartMotor::~SmartMotor(void)
{
}

// Initialise all pins
void SmartMotor::init()
{
	Serial1.begin(BAUDRATE);

	pinMode(RX_ENABLE, OUTPUT);
	pinMode(DX_ENABLE, OUTPUT);
	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	return;
}

// Startup sequence for motor
void SmartMotor::startup()
{
	sendMessage("ZS");	// Clear Flags
	sendMessage("EIGN(2)");
	sendMessage("EIGN(3)");
	sendMessage("ZS");	// Clear Flags
	sendMessage("MP");	// Position Mode
}

//=====================================
//              SETTERS
//=====================================

void SmartMotor::setPosition(int32_t value)
{
	//char posn[MAX_PACKET_SIZE] = "PT=";
	//itoa(value, posn+3, 10);
	//sendMessage(posn);
	char msg[MAX_PACKET_SIZE];
	sprintf(msg, "PT=%ld",value);
	sendMessage(msg);
}
void SmartMotor::setPositionRelative(int32_t value)
{
	//char posn[MAX_PACKET_SIZE] = "PT=";
	//itoa(value, posn+3, 10);
	//sendMessage(posn);
	char msg[MAX_PACKET_SIZE];
	sprintf(msg, "PRT=%ld",value);
	sendMessage(msg);
}
void SmartMotor::setVelocity(int32_t value)
{
	//char vel[MAX_PACKET_SIZE] = "VT=";
	//itoa(value, vel+3, 10);
	//sendMessage(vel);
	char msg[MAX_PACKET_SIZE];
	sprintf(msg, "VT=%ld",value);
	sendMessage(msg);
}
void SmartMotor::setAcceleration(int32_t value)
{
	//char acc[MAX_PACKET_SIZE] = "ADT=";
	//itoa(value, acc+4, 10);
	char msg[MAX_PACKET_SIZE];
	sprintf(msg, "ADT=%ld",value);
	sendMessage(msg);
}
void SmartMotor::setRun(bool run_status)
{
	if(run_status)
	{
		sendMessage("G");
	}
	else
	{
		sendMessage("S");		
	}
}

//=====================================
//              GETTERS
//=====================================

void SmartMotor::requestPosition()
{
	sendMessage("RPA");
	request_type = REQUEST_POSITION;
}

void SmartMotor::requestGoal()
{
	sendMessage("RPT");
	request_type = REQUEST_GOAL;
}
void SmartMotor::requestVelocity()
{
	sendMessage("RVT");
	request_type = REQUEST_VELOCITY;
}
void SmartMotor::requestAcceleration()
{
	sendMessage("RAT");
	request_type = REQUEST_ACCELERATION;
}
void SmartMotor::requestTrajectoryFlag()
{
	sendMessage("RBt");
	request_type = REQUEST_TRAJECTORY_FLAG;
}


//=====================================
//              GETTERS
//=====================================
int32_t SmartMotor::getPosition()
{
	return _motorState.position;
}
int32_t SmartMotor::getGoal()
{
	return _motorState.goal;
}
int32_t SmartMotor::getVelocity()
{
	return _motorState.velocity;
}
int32_t SmartMotor::getAcceleration()
{
	return _motorState.acceleration;
}
int8_t SmartMotor::getTrajectoryFlag()
{
	return _motorState.trajectory_flag;
}


//=================================
//       READ WRITE FUNCTIONS
//=================================

bool SmartMotor::readMessage()//AvailableCallback fAvailable, ReadCallback fRead)
{
	if (Serial1.available())
	{
		//Serial.println("Available");
		char input = Serial1.read();

			data[len] = input;
			//Serial.print(input);
			if (len >= MAX_PACKET_SIZE)
			{
				len = 0;
				return false;
			}
			if (input == PACKET_END_RX)	// End of received packet
			{
				data[len] = '\0';
				//data_integer = (int)data[len-1];
				//for (int i = (len-1); i > 0; i--) {
				//	data_integer = data_integer | ((int)data[i-1] << ((len-i) * 8));
				//}
				//data_integer = (int32_t)data[0];
				//for (int i = 1; i < len; i++) {
				//	data_integer = data_integer | ((int32_t)data[i] << ((i) * 8));
				//}
				decodeMessage(); //Need to check CRC
				len = 0;
				//Serial.println("DONE");
				return true;
			}
			len++;

		return true;
	}
	else
	{
		return false;
	}
}

void SmartMotor::sendMessage(char msg[])//, WriteCallback fWrite, FlushCallback fflush)
{
	Serial1.flush();

	digitalWrite(RX_ENABLE, HIGH);  // disable receiving
	digitalWrite(DX_ENABLE, HIGH);  // enable sending

	for(int i = 0; msg[i] != '\0'; i++)
	{
		// for DEBUG
		//Serial.println(data[i], HEX);
		Serial1.write(msg[i]);
		//Serial.print(msg[i]);
	}
	//Serial.println("");
	
	Serial1.write(PACKET_END_TX);
	Serial1.flush();
	//delay(1); // Delay abit or else the RS485 will switch off before the last byte has been sent.

	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	return;
}

void SmartMotor::decodeMessage()
{
	auto to_int32 = [] (byte *data, int len) {
		int sign;
		int32_t out;
		if (data[0] == '-') {
			sign = -1;
			out = 0;
		}
		else {
			sign = 1;
			out = int(data[0]) - int('0');
		}
		
		for (int i = 1; i < len; i++)
			out = (out * 10) + (int(data[i]) - int('0'));

		out *= sign;

		return out;
	};

	switch (request_type){
	case REQUEST_POSITION:
		_motorState.position = to_int32(data, len);
		Serial.println(_motorState.position);
		break;

	case REQUEST_GOAL:
		_motorState.goal = to_int32(data, len);
		Serial.println(_motorState.goal);
		break;

	case REQUEST_VELOCITY:
		_motorState.velocity = to_int32(data, len);
		break;

	case REQUEST_ACCELERATION:
		_motorState.acceleration = to_int32(data, len);
		break;

	case REQUEST_TRAJECTORY_FLAG:
		_motorState.trajectory_flag = to_int32(data, len);
		Serial.println(_motorState.trajectory_flag);
		break;

	default:
		//Error
		break;
	}
}