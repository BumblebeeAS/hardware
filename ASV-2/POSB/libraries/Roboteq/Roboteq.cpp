
#include "Roboteq.h"

#include "Arduino.h"

Roboteq::Roboteq(MCP_CAN *canptr) : CAN(canptr)
{
}

void Roboteq::init()
{
}

//******** DECODE REPLY ***********

uint16_t Roboteq::getReplyIndex()
{
	uint16_t index = buf[1] + (buf[2] << 8);
	return index;
}

void Roboteq::decodeReply(uint16_t index)
{
	switch (index)
	{
	case INDEX_READ_MOTOR_AMPS:
		stats.motor_current = CAN->parseCANFrame(buf, 4, 2);
		Serial.print("Motor current: "); 
		Serial.println(stats.motor_current);
		break;
	case INDEX_READ_ACTUAL_MOTOR_COMMAND:
		stats.motor_comand1 = CAN->parseCANFrame(buf, 4, 2);
		Serial.print("Motor command: ");
		Serial.println(stats.motor_comand1);
		break;
	case INDEX_READ_BATTERY_AMPS:
		stats.battery_current = CAN->parseCANFrame(buf, 4, 2);
		for (int i = 0; i < 8; i++)
		{
			Serial.print(buf[i], HEX);
			Serial.print("|");
		}
		Serial.print("Battery current: ");
		Serial.println(stats.battery_current);
		break;
  case INDEX_READ_BATTERY_VOLTS:
    stats.battery_volt = CAN->parseCANFrame(buf, 4, 2);
    Serial.print("Battery volt: ");
    Serial.println(stats.battery_volt);
    break;
	}
}

void Roboteq::sendCANmsg(uint16_t index, INT8U subidx, INT8U ccs, INT8U len, INT32U data)
{
	INT8U byte0;
	INT8U lenempty;
	lenempty = 4 - len;
	byte0 = (lenempty << 2) + (ccs << 4);

	CAN->setupCANFrame(buf, 0, 1, byte0);	// Populate byte0 
	CAN->setupCANFrame(buf, 1, 2, index);	// Populate index
	CAN->setupCANFrame(buf, 3, 1, subidx);	// Populate subindex
	CAN->setupCANFrame(buf, 4, 4, data);	// Populate data
	/*Serial.print("ID:");
	Serial.print(ROBOTEQ_CAN1_SEND_INDEX,HEX);
	Serial.print("LEN:");
	Serial.println(len);
	for (int i = 0; i < 8; i++)
	{
		Serial.print(buf[i],HEX);
		Serial.print("|");
	}
	Serial.println("");*/
	
	CAN->sendMsgBuf(ROBOTEQ_CAN1_SEND_INDEX, 0, len+4, buf);
}


void Roboteq::setMotorSpeed(int32_t speed, uint8_t channel)
{
	Serial.print("Set speed (");
	Serial.print(channel);
	Serial.print("): ");
	Serial.println(speed);
	sendCANmsg(INDEX_SET_MOTOR, channel, CCS_COMMAND, 4, speed);
}

void Roboteq::kill()
{
	Serial.println("Kill");
	sendCANmsg(INDEX_EMERGENCY_SHUTDOWN, 0, CCS_COMMAND, 1, 0);
}

void Roboteq::unkill()
{
	Serial.println("Unkill");
	sendCANmsg(INDEX_RELAESE_SHUTDOWN, 0, CCS_COMMAND, 1, 0);
}

void Roboteq::requestBatteryVolts()
{
  Serial.println("Request Battery Volts...");
  sendCANmsg(INDEX_READ_BATTERY_VOLTS, 2, CCS_QUERY, 2, 0x0);
}

void Roboteq::requestBatteryAmps()
{
	Serial.println("Request Battery Amps...");
	sendCANmsg(INDEX_READ_BATTERY_AMPS, 1, CCS_QUERY, 2, 0x0);
}

void Roboteq::requestMotorAmps()
{
	Serial.println("Request Motor Amps...");
	sendCANmsg(INDEX_READ_MOTOR_AMPS, 1, CCS_QUERY, 2, 0x0);
}

void Roboteq::requestMotorCommand()
{
	Serial.println("Request Motor Command...");
	sendCANmsg(INDEX_READ_ACTUAL_MOTOR_COMMAND, 1, CCS_QUERY, 2, 0x0);
}

void Roboteq::readRoboteqReply()
{
	uint16_t index;
 int i;
	CAN->readMsgBufID(&id, &len, buf);// read data,  len: data length, buf: data buf
	/*Serial.print("ID: ");
	Serial.print(id, HEX);
	Serial.print("\tDATA: ");
  for(i = 0; i < 8; i++)
  {
    Serial.print(buf[i],HEX);
	Serial.print(" | ");
  }*/

	if (id == ROBOTEQ_CAN1_REPLY_INDEX)
	{
		index = getReplyIndex();
		decodeReply(index);
	}
}
