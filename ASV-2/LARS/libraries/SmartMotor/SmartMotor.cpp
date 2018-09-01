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
	sendMessage("RPT");
	request_type = REQUEST_POSITION;
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

//=====================================
//              GETTERS
//=====================================



//=================================
//       READ WRITE FUNCTIONS
//=================================

bool SmartMotor::readMessage()//AvailableCallback fAvailable, ReadCallback fRead)
{
	if (Serial1.available())
	{
		char input = Serial1.read();

			data[len] = input;
			len++;
			Serial.print(input);
			if (len >= MAX_PACKET_SIZE)
			{
				len = 0;
				return false;
			}
			if (input == PACKET_END_RX)	// End of received packet
			{
				data[len] = '\0';
				len = 0;
				//decodeMessage(); //Need to check CRC
				Serial.println("DONE");
				return true;
			}
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
		Serial.print(msg[i]);
	}
	Serial.println("");
	
	Serial1.write(PACKET_END_TX);
	Serial1.flush();
	//delay(1); // Delay abit or else the RS485 will switch off before the last byte has been sent.

	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	return;
}