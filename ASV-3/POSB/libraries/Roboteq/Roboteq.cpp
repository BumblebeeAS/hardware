
#include "Roboteq.h"

#include "Arduino.h"

Roboteq::Roboteq(MCP_CAN *canptr, uint16_t can_send) : CAN(canptr)
{
	_id = 0;
	_len = 0;
	if (can_send == 1)
	{
		can_send_idx = ROBOTEQ_CAN1_SEND_INDEX;
		can_reply_idx = ROBOTEQ_CAN1_REPLY_INDEX;
	}
	else
	{
		can_send_idx = ROBOTEQ_CAN2_SEND_INDEX;
		can_reply_idx = ROBOTEQ_CAN2_REPLY_INDEX;
	}
}

void Roboteq::init()
{
}

//********** GETTER ***************


RoboteqStats Roboteq::getRoboteqStats()
{
	return stats;
}

//******** DECODE REPLY ***********

uint16_t Roboteq::getReplyIndex()
{
	uint16_t index = _buf[1] + (_buf[2] << 8);
	return index;
}

uint8_t Roboteq::getSubindex()
{
	return CAN->parseCANFrame(_buf, 3, 1);
}

void Roboteq::decodeReply(uint16_t index)
{
	switch (index)
	{
	case INDEX_READ_MOTOR_AMPS:
		if (getSubindex() == 1)
		{
			stats.motor_current1 = CAN->parseCANFrame(_buf, 4, 2);
		//Serial.print("Motor current1: "); 
		//Serial.println(stats.motor_current1);
		}
		else
		{
			stats.motor_current2 = CAN->parseCANFrame(_buf, 4, 2);
		//Serial.print("Motor current2: "); 
		//Serial.println(stats.motor_current2);
		}
		break;
	case INDEX_READ_FAULT_FLAGS:
		stats.fault_flags = CAN->parseCANFrame(_buf, 4, 1);
		//Serial.print("Fault flags: ");
		//Serial.println(stats.fault_flags, BIN);
		break;
	case INDEX_READ_MOTOR_STATUS_FLAGS:
		if (getSubindex() == 1)
		{
			stats.motor_status_flags1 = CAN->parseCANFrame(_buf, 4, 1);
			//Serial.print("Motor status flag 1: ");
			//Serial.println(stats.motor_status_flags1, BIN);
		}
		else
		{
			stats.motor_status_flags2 = CAN->parseCANFrame(_buf, 4, 1);
			//Serial.print("Motor status flag 2: ");
			//Serial.println(stats.motor_status_flags2, BIN);
		}
		break;
	case INDEX_READ_ACTUAL_MOTOR_COMMAND:
		stats.motor_comand1 = CAN->parseCANFrame(_buf, 4, 2);
		Serial.print("Motor command: ");
		Serial.println(stats.motor_comand1);
		break;
	case INDEX_READ_BATTERY_AMPS:
		stats.battery_current = CAN->parseCANFrame(_buf, 4, 2);
		for (int i = 0; i < 8; i++)
		{
			Serial.print(_buf[i], HEX);
			Serial.print("|");
		}
		Serial.print("Battery current: ");
		Serial.println(stats.battery_current);
		break;
	case INDEX_READ_BATTERY_VOLTS:
		stats.battery_volt = CAN->parseCANFrame(_buf, 4, 2);
		Serial.print("Battery volt: ");
		Serial.println(stats.battery_volt);
		break;
	}
}

//******** SEND QUERY ***********

void Roboteq::requestUpdate()
{
	msg_type++; //change message type
	msg_type %= 5;
	//Serial.println("UPDATE");
	switch (msg_type) {
	case 0:
		requestMotorAmps(1);
		break;
	case 1:
		requestMotorAmps(2);
		break;
	case 2:
		requestMotorStatusFlags(1);
		break;
	case 3:
		requestMotorStatusFlags(2);
		break;
	case 4:
		requestFaultFlags();
		break;
	default:
		//Error
		break;
	}
}

void Roboteq::setMotorSpeed(int32_t speed, uint8_t channel)
{
	/*
	Serial.print("Set speed (");
	Serial.print(channel);
	Serial.print("): ");
	Serial.println(speed);*/
	sendCANmsg(INDEX_SET_MOTOR, channel, CCS_COMMAND, 4, speed);
}

void Roboteq::requestMotorAmps(uint8_t ch)
{
#ifdef _ESC_DEBUG_
	Serial.println("Request Motor Amps...");
#endif
	sendCANmsg(INDEX_READ_MOTOR_AMPS, ch, CCS_QUERY, 4, 0x0);
}

void Roboteq::requestFaultFlags()
{
#ifdef _ESC_DEBUG_
	Serial.println("Request Fault Flags...");
#endif
	sendCANmsg(INDEX_READ_FAULT_FLAGS, 0, CCS_QUERY, 2, 0x0);
}

void Roboteq::requestMotorStatusFlags(uint8_t ch)
{
#ifdef _ESC_DEBUG_
	Serial.print("Request Motor Status Flags...");
	Serial.println(ch);
#endif
	sendCANmsg(INDEX_READ_MOTOR_STATUS_FLAGS, ch, CCS_QUERY, 2, 0x0);
}


//******** READ / SEND CAN ***********

void Roboteq::sendCANmsg(uint16_t index, INT8U subidx, INT8U ccs, INT8U len, INT32U data)
{
	INT8U byte0;
	INT8U lenempty;
	lenempty = 4 - len;
	byte0 = (lenempty << 2) + (ccs << 4);

	CAN->setupCANFrame(_buf, 0, 1, byte0);	// Populate byte0 
	CAN->setupCANFrame(_buf, 1, 2, index);	// Populate index
	CAN->setupCANFrame(_buf, 3, 1, subidx);	// Populate subindex
	CAN->setupCANFrame(_buf, 4, 4, data);	// Populate data
											/*
											Serial.print("ID:");
											Serial.print(ROBOTEQ_CAN1_SEND_INDEX,HEX);
											Serial.print("LEN:");
											Serial.println(len);
											for (int i = 0; i < 8; i++)
											{
											Serial.print(_buf[i],HEX);
											Serial.print("|");
											}
											Serial.println("");*/
	CAN->sendMsgBuf(can_send_idx, 0, len + 4, _buf);
}


void Roboteq::readRoboteqReply_fromCAN()
{
	int i;
	CAN->readMsgBufID(&_id, &_len, _buf);// read data,  len: data length, buf: data buf
	/*Serial.print("ID: ");
	Serial.print(id, HEX);
	Serial.print("\tDATA: ");
	for(i = 0; i < 8; i++)
	{
	Serial.print(buf[i],HEX);
	Serial.print(" | ");
	}*/

	if (_id == can_reply_idx)
	{
		readRoboteqReply(_id, _len, _buf);
	}
}

void Roboteq::readRoboteqReply(uint32_t id, uint8_t len, uint8_t *buf)
{
	uint16_t index;
	_id = id;
	_len = len;

	// copy buf over
	for(int i = 0; i < len; i++)
	{
		_buf[i] = buf[i];
#ifdef _ESC_DEBUG_
		if(buf[0] != 0x60)
		{
			Serial.print(buf[i],HEX);
			Serial.print(" ");
		}
#endif
	}

#ifdef _ESC_DEBUG_
		if(buf[0] != 0x60)
			Serial.println("");
#endif

	index = getReplyIndex();
	decodeReply(index);
}

// ================================
//			Unused Commands
// ================================


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

void Roboteq::requestMotorCommand()
{
	Serial.println("Request Motor Command...");
	sendCANmsg(INDEX_READ_ACTUAL_MOTOR_COMMAND, 1, CCS_QUERY, 2, 0x0);
}