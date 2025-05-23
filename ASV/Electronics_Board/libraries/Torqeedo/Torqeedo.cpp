#include "Torqeedo.h"
#include <Arduino.h>

Torqeedo::Torqeedo(int RXEN, int DXEN, int ON, int thruster_num)
{
	RX_ENABLE = RXEN;
	DX_ENABLE = DXEN;
	ON_PIN = ON;
	thrusterNum = thruster_num;
	len = 0;
	setMotorDrive(0);
}

Torqeedo::~Torqeedo(void)
{
}

void Torqeedo::init()
{
	if (thrusterNum == 1)
	{
		Serial1.begin(BAUDRATE);
		_write = write1;
		_available = available1;
		_read = read1;
		_flush = flush1;
	}
	else
	{
		Serial2.begin(BAUDRATE);
		_write = write2;
		_available = available2;
		_read = read2;
		_flush = flush2;
	}

	pinMode(RX_ENABLE, OUTPUT);
	pinMode(DX_ENABLE, OUTPUT);
	pinMode(ON_PIN, OUTPUT);
	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	//digitalWrite(ON_PIN, LOW);  // on pin float
	onThruster(true);
	return;
}

void Torqeedo::onThruster(bool on_status)
{
	setMotorDrive(0);
	startUpCount = 0;
	digitalWrite(ON_PIN, HIGH);  // on thruster
	if (on_status)
	{
		onPinPeriod = THRUSTERON;
	}
	else
	{
		onPinPeriod = THRUSTEROFF;
	}
	onTime = millis();
	powerSeq = true;
	Serial.println("ON");
}
bool Torqeedo::checkThrusterOnOff()
{
	if (powerSeq && (millis() - onTime > onPinPeriod))
	{
		digitalWrite(ON_PIN, LOW);
		powerSeq = false;
		Serial.println("OFF");
		return true;
	}
	return false;
}

//=====================================
//              GETTERS
//=====================================
DisplayState Torqeedo::getDisplayState()
{
	return _displayState;
}

uint8_t* Torqeedo::getMotorstats()
{
	return motorstats;
}
uint8_t* Torqeedo::getBatterystats()
{
	return batterystats;
}
uint8_t* Torqeedo::getRangestats()
{
	return rangestats;
}
bool Torqeedo::getMotorHeartbeat()
{
	return motorReply;
}

//=====================================
//         SET PACKAGE MESSAGE
//=====================================
bool Torqeedo::setMotorDrive(int speed)
{
	_motorDrive.motor_speed = mapSpeed(speed);
#ifdef DEBUG
	_motorDrive.motor_speed = (int16_t)hardcodespeed;
#endif
	if ((_motorDrive.motor_speed > 1000) || (_motorDrive.motor_speed < -1000))
	{
		_motorDrive.motor_speed = 0;
	}

	if (kill)
	{
		//E-stop
		_motorDrive.body[0] = 0x04;
		_motorDrive.body[1] = 0x00;
		_motorDrive.body[2] = 0x00;
		_motorDrive.body[3] = 0x00;
	}
	else if (speedZero)
	{
		//Motorspeed valid but 0
		_motorDrive.body[0] = 0x05;
		_motorDrive.body[1] = 0x00;
		_motorDrive.body[2] = 0x00;
		_motorDrive.body[3] = 0x00;
	}
	else
	{
		_motorDrive.body[0] = 0x05;
		_motorDrive.body[1] = 0x00;
		_motorDrive.body[2] = _motorDrive.motor_speed >> 8;
		_motorDrive.body[3] = _motorDrive.motor_speed;
	}
	_motorDrive.body[4] = END_MARKER;

	return true;
}

void Torqeedo::setKill(bool kill_status)
{
	kill = kill_status;
	return;
}

#ifdef DEBUG
int speedcount = 0;
const int speedarrsize = 220;
int16_t speedarr[speedarrsize] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,300,300,800,800,
	300,300,800,800,
};
#endif


//=================================
//      DECODE MESSAGE_ID
//=================================

bool Torqeedo::decodeMessage()
{
	byte address = data[0];

	/*
	int8_t crc = 0x00;
	for (int i = 0; data[i] != 0xFF; i++)
	{
		crc = crc8(crc, data[i]);
	}

	if (_checksum != crc)
		return false;
		*/

	if (address != DeviceId_Master)
	{
		if (motorRequest)
			motorReply = false;
		motorRequest = false;
	}

	switch (address)
	{
	case DeviceId_Display:
		decodeDisplay();
		//sendEmptyReply();
		//Serial.print("display");
		break;

	case DeviceId_Remote1:
		if (data[1] == REMOTE_Remote)
		{
			if (startUpCount < 20)
			{
				speedZero = true;
				startUpCount++;
			}
			else
			{
				speedZero = false;
			}
			/*
			for (int i = 0; i < 4; i++)
			{
				Serial.print(_motorDrive.body[i], HEX);
				Serial.print(" ");
			}
			Serial.print("\n");*/
			sendMessage(_motorDrive.body);
#ifdef DEBUG			
			//Use speedarr to determine speed
			if (speedcount >= speedarrsize)
				speedy = 0;
			else
				speedy = speedarr[speedcount];
			speedcount++;
			//speedcount = 0;						
#endif
		}
		break;
	case DeviceId_Motor:
		motorRequest = true;
		break;
	case DeviceId_Master:
		if (motorRequest)
		{
			motorReply = true;
			motorRequest = false;
		}
		break;
	}

	return true;
}


void Torqeedo::decodeDisplay()
{
	byte messageID = data[1];

	switch (messageID)
	{
	case DISPLAY_SystemState:
		/*
		Serial.print("\nRAW ::: ");
		for (int i = 0; i < 26; i++)
		{
			Serial.print(data[i], HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
		*/
		decodeDisplayState();
		/*
		Serial.println("\nDISPLAY");
		for (int i = 0; i < 8; i++)
		{
			Serial.print(motorstats[i], HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
		for (int i = 0; i < 6; i++)
		{
			Serial.print(batterystats[i], HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
		for (int i = 0; i < 6; i++)
		{
			Serial.print(rangestats[i], HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
		Serial.print("Motor: (V) ");
		Serial.print((int)(uint16_t)(motorstats[1] << 8 | motorstats[0]));
		Serial.print(" (A) ");
		Serial.print((int)(uint16_t)(motorstats[3] << 8 | motorstats[2]));
		Serial.print(" (S) ");
		Serial.println((int16_t)(motorstats[5] << 8 | motorstats[4]));
		Serial.print("Temp: (Motor) ");
		Serial.println((int)(uint8_t)(motorstats[7]));
		Serial.print(" (Master) ");
		Serial.println((int)(uint8_t)(batterystats[5]));
		Serial.print("Batt: (%) ");
		Serial.print((int)(uint16_t)(batterystats[0]));
		Serial.print(" (V) ");
		Serial.print((int)(uint16_t)(batterystats[2] << 8 | batterystats[1]));
		Serial.print(" (A) ");
		Serial.println((int)(uint16_t)(batterystats[4] << 8 | batterystats[3]));
		Serial.print("Range: (GPS) ");
		Serial.print((int)(uint16_t)(rangestats[1] << 8 | rangestats[0]));
		Serial.print(" (Miles) ");
		Serial.print((int)(uint16_t)(rangestats[3] << 8 | rangestats[2]));
		Serial.print(" (Min) ");
		Serial.println((int)(uint16_t)(rangestats[5] << 8 | rangestats[4]));
		*/
#ifdef _TEST_
		Serial.print("Batt");
		Serial.print(thrusterNum);
		Serial.print("(%): ");
		Serial.print((int)(uint16_t)(batterystats[0]));
		Serial.print(" Speed");
		Serial.print(thrusterNum);
		Serial.print(": ");
		Serial.println(_motorDrive.motor_speed);
#endif

		break;
	case DISPLAY_SystemSetup:
		break;
	}
	return;
}


//=================================
//      DECODE PACKET MESSAGE
//=================================

void Torqeedo::decodeDisplayState()
{
	byte *ptr = data + 2;

	// Motor voltage - uint16
	motorstats[0] = ptr[4];
	motorstats[1] = ptr[5];
	// Motor current - uint16
	motorstats[2] = ptr[6];
	motorstats[3] = ptr[7];
	// Motor speed - sint16
	uint16_t speedstat = ptr[11] << 8 | ptr[10];
	speedstat += 0x80;
	motorstats[4] = speedstat >> 8;
	motorstats[5] = speedstat;
	// PCB Temp - unit8
	motorstats[6] = ptr[12];
	// Motor Flags
	motorstats[7] = ptr[1];

	// Battery Charge - unit8
	batterystats[0] = ptr[14];
	// Battery Voltage - unit16
	batterystats[1] = ptr[15];
	batterystats[2] = ptr[16];
	// Battery Current - unit16
	batterystats[3] = ptr[17];
	batterystats[4] = ptr[18];
	// Master PCB Temp SW - uint8
	batterystats[5] = ptr[25];

	// GPS Speed - unit16
	rangestats[0] = ptr[19];
	rangestats[1] = ptr[20];
	// Range miles - unit16
	rangestats[2] = ptr[21];
	rangestats[3] = ptr[22];
	// Range Minutes - unit16
	rangestats[4] = ptr[23];
	rangestats[5] = ptr[24];
	/*
	_displayState.motor_voltage = (uint16_t)*(ptr + 4);
	_displayState.motor_current = (uint16_t)*(ptr + 6);
	_displayState.motor_power = (uint16_t)*(ptr + 8);
	_displayState.motor_speed = (int16_t)*(ptr + 10);
	_displayState.battery_charge = (uint8_t)*(ptr + 14);
	_displayState.battery_voltage = (uint16_t)*(ptr + 15);
	_displayState.battery_current = (uint16_t)*(ptr + 17);
	*/

	return;
}

//=================================
//       READ WRITE FUNCTIONS
//=================================

void Torqeedo::sendEmptyReply()
{
	byte emptyData[1] = { 0xFF };
	sendMessage(emptyData);// , _write);
}

void Torqeedo::EncodeMessage(byte body[])
{
	int dataidx = 3;
	int bodyidx = 0;

	data[0] = PACKET_START;
	data[1] = 0x00;
	data[2] = 0x00;

	int8_t crc = 0;
	while (body[bodyidx] != END_MARKER)
	{
		crc = crc8(crc, body[bodyidx]);
		data[dataidx] = body[bodyidx];
		bodyidx++;
		dataidx++;
	}
	data[dataidx++] = crc;
	data[dataidx++] = PACKET_END;
	data[dataidx] = END_MARKER;

	return;
}

bool Torqeedo::sendMessage(byte body[])//, WriteCallback fWrite, FlushCallback fflush)
{
	_flush();

	digitalWrite(RX_ENABLE, HIGH);  // disable receiving
	digitalWrite(DX_ENABLE, HIGH);  // enable sending
	EncodeMessage(body);

	int counter = 0;
	//time = millis();
	//data[0] = 0x01;
	//data[1] = END_MARKER;
	_write(0xFF);
	while (data[counter] != END_MARKER)
	{
		// for DEBUG
		//Serial.println(data[counter], HEX);
		_write(data[counter]);
		counter++;
	}
	_write(0xFF);
	_flush();
	//delay(1); // Delay abit or else the RS485 will switch off before the last byte has been sent.

	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	return true;
}

bool Torqeedo::readMessage()//AvailableCallback fAvailable, ReadCallback fRead)
{
	if (_available())
	{
		byte input = _read();
		switch (input)
		{
			//If not PACKET_START, skip message
		case PACKET_START:
			msgStart = true;
			len = 0;
			break;

		case PACKET_END:
			msgStart = false;
			len--;
			_checksum = data[len];
			data[len] = 0xFF;
			len = 0;
			decodeMessage(); //Need to check CRC
			break;

		default:
			if (!msgStart)
				break;
			else
			{
				data[len] = input;
				len++;
				if (len >= MAX_PACKET_SIZE)
				{
					len = 0;
					msgStart = false;
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


//=========================================
//            UTILITIES
//=========================================


int16_t Torqeedo::mapSpeed(int speed)
{
	int16_t mapped;
	if (speed == 0)
		mapped = 0;
	else if (speed > 0)
		mapped = map(speed, 0, 1000, 55, 1000);
	else
		mapped = map(speed, -1000, -1, -1000, -55);
	return mapped;
}

//TODO: Check if CRC is correct
#define CRC8INIT 0x00
#define CRC8POLY 0x31 // = X^8+X^5+X^4+X^0

int8_t Torqeedo::crc8(int8_t crc, int8_t crc_data)
{
	int8_t i;
	i = (crc_data ^ crc) & 0xff;
	crc = 0;
	if (i & 1)
		crc ^= 0x5e;
	if (i & 2)
		crc ^= 0xbc;
	if (i & 4)
		crc ^= 0x61;
	if (i & 8)
		crc ^= 0xc2;
	if (i & 0x10)
		crc ^= 0x9d;
	if (i & 0x20)
		crc ^= 0x23;
	if (i & 0x40)
		crc ^= 0x46;
	if (i & 0x80)
		crc ^= 0x8c;
	return(crc);
}

void write1(byte content)
{
	Serial1.write(content);
	return;
}
byte read1()
{
	byte input = (byte)Serial1.read();
	/*
	Serial.print(input, HEX);
	Serial.print(" ");
	if (input == 0xAD)
		Serial.print("\r\n");
		*/
	return input;
}
int available1()
{
	return Serial1.available();
}
void flush1()
{
	Serial1.flush();
	return;
}

void write2(byte content)
{
	Serial2.write(content);
	return;
}
byte read2()
{
	byte input = (byte)Serial2.read();
	/*
	Serial.print(input, HEX);
	Serial.print(" ");
	if (input == 0xAD)
	Serial.print("\r\n");
	*/
	return input;
}
int available2()
{
	return Serial2.available();
}
void flush2()
{
	Serial2.flush();
	return;
}