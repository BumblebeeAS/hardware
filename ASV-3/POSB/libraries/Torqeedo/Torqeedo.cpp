#include "Arduino.h"
#include "Torqeedo.h"

int Torqeedo::powerSeq = POWERSEQ_OFF;

Torqeedo::Torqeedo(int RXEN, int DXEN, int ON, int batt_num)
{
	RX_ENABLE = RXEN;
	DX_ENABLE = DXEN;
	ON_PIN = ON;
	_battNum = batt_num;
	len = 0;
	//requestData(0);
}

Torqeedo::~Torqeedo(void)
{
}

void Torqeedo::init()
{
	if (_battNum == 1)
	{
		Serial1.begin(BAUDRATE);
		_write = write1;
		_available = available1;
		_read = read1;
		_flush = flush1;
	}
	else
	{
		Serial3.begin(BAUDRATE);
		_write = write2;
		_available = available2;
		_read = read2;
		_flush = flush2;
	}

	pinMode(RX_ENABLE, OUTPUT);
	pinMode(DX_ENABLE, OUTPUT);
	pinMode(ON_PIN, OUTPUT);
	
	{
		delay(2000);
		digitalWrite(ON_PIN, LOW);
		delay(1000);
		digitalWrite(ON_PIN, HIGH);
	}
	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving
	//digitalWrite(ON_PIN, LOW);  // on pin float
	onBattery(true);
	return;
}

void Torqeedo::onBattery(bool on_status)
{
	if (powerSeq == POWERSEQ_OFF)
	{
		digitalWrite(ON_PIN, HIGH);  // on battery
		if (on_status)
		{
			onPinPeriod = BATTERY_ON_DURATION;
		}
		else
		{
			onPinPeriod = BATTERY_OFF_DURATION;
		}
		onTime = millis();
		powerSeq = _battNum;
	}
	//Serial.println("ON");
}

// Check if on pin is being pulled up
// Pull back down when time is up
bool Torqeedo::checkBatteryOnOff()
{
	if ((powerSeq == _battNum) && (millis() - onTime > onPinPeriod))
		// Check if I am the batt that initiate powerseq
		// If yes, check how long since powerseq start
	{
		//if turning on, trigger 1 message packet to enable comms
		if (onPinPeriod == BATTERY_ON_DURATION) {
			requestUpdate();
		}
		digitalWrite(ON_PIN, LOW);
		powerSeq = POWERSEQ_OFF;
		requestCount=0;
		return true;
	}
	return false;
}

// Re-trigger startup if no response received for BATT_RESET_COUNT times
void Torqeedo::checkBatteryConnected()
{
	if ((requestCount > BATT_RESET_COUNT) && (powerSeq == POWERSEQ_OFF)){
		Serial.print("Restart batt comms... ");
		Serial.println(_battNum);
		requestCount = 0;
		onBattery(true);
	}
	return;
}

void Torqeedo::requestUpdate(){
	byte body[5] = {0};
	body[0] = DeviceId_Battery;
	body[2] = END_MARKER;
	msg_type++; //change message type
	msg_type %= 3;
	//Serial.println("UPDATE");
	switch (msg_type){
	case MSG_STATUS:
		body[1] = BATTERY_STATUS_ID;
		break;

	case MSG_TEMP:
		body[1] = BATTERY_TEMPS_ID;
		break;

	case MSG_VOLT:
		body[1] = BATTERY_VOLTS_ID;
		break;

	default:
		//Error
		break;
	}

	//Hard coded for testing
	//body[1] = BATTERY_STATUS_ID;

	sendMessage(body);
}

//=====================================
//              GETTERS
//=====================================

void Torqeedo::getData(){

}

void Torqeedo::resetData() {
	battData.voltage = 255;
	battData.battCurrent = -255;
	battData.capPercent = 255;
	battData.cellMaxTemp = 255;
}

uint16_t Torqeedo::getVoltage(){
	return battData.voltage;
}

int16_t Torqeedo::getCurrent() {
	return battData.battCurrent;
}

uint16_t Torqeedo::getCapacity() {
	return battData.capPercent;
}

int16_t Torqeedo::getTemperature() {
	return battData.cellMaxTemp;
}

//=================================
//      DECODE MESSAGE
//=================================
/*
 * Checks CRC and Decodes message data
 * Returns true is message is valid, false otherwise
 */
bool Torqeedo::decodeMessage()
{

	uint8_t crc = 0;
	int i;
	for (i = 0; data[i] != 0xAD; i++)
	{
		crc = crc8(crc, data[i]);
		//Serial.print(data[i]);
		//Serial.print(" ");
	}

	if (_checksum != crc)
	{
		Serial.println("Bad CS");
		return false;
	}

	switch (msg_type){
	case MSG_STATUS:
		battData.statusFlag = data[2] << 8 | data[3];
		battData.warningFlag = data[4] << 8 | data[5];
		battData.errorFlag = data[6] << 8 | data[7];
		battData.cellMaxTemp = data[8] << 8 | data[9];
		battData.pcbMaxTemp = data[10] << 8 | data[11];
		battData.voltage = data[12] << 8 | data[13];
		battData.battCurrent = data[14] << 8 | data[15];
		battData.powerDrawn = data[16] << 8 | data[17];
		battData.maxLearnedCap = data[18] << 8 | data[19];
		battData.capRemaining = data[20] << 8 | data[21];
		battData.capPercent = data[22] << 8 | data[23];
		battData.timeLeft = data[24] << 8 | data[25];
		battData.waterSense = data[26] << 8 | data[27];
		break;

	case MSG_TEMP:
		battData.tempCell1 = data[2] << 8 | data[3];
		battData.tempCell2 = data[4] << 8 | data[5];
		battData.tempPCB1 = data[12] << 8 | data[13];
		battData.tempPCB2 = data[14] << 8 | data[15];
		break;

	case MSG_VOLT:
		battData.voltCell1 = data[2] << 8 | data[3];
		battData.voltCell2 = data[4] << 8 | data[5];
		battData.voltCell3 = data[6] << 8 | data[7];
		battData.voltCell4 = data[8] << 8 | data[9];
		battData.voltCell5 = data[10] << 8 | data[11];
		battData.voltCell6 = data[12] << 8 | data[13];
		battData.voltCell7 = data[14] << 8 | data[15];

		break;

	default:
		//Error
		break;
	}

	// To check for battery connection
//	if (requestCount > 0)
//		requestCount--;
//	else
//		requestCount = 0;
	requestCount = 0;

	return true;
}


//=================================
//      DECODE PACKET MESSAGE
//=================================


//=================================
//      ENCODE MESSAGE
//=================================
/*
 * Encode message data and append CRC
 */

void Torqeedo::encodeMessage(byte body[])
{
	int dataidx = 1;
	int bodyidx = 0;

	data[0] = PACKET_START;

	uint8_t crc = 0;
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

//=================================
//       READ WRITE FUNCTIONS
//=================================


bool Torqeedo::sendMessage(byte body[])//, WriteCallback fWrite, FlushCallback fflush)
{
	_flush();

	digitalWrite(RX_ENABLE, HIGH);  // disable receiving
	digitalWrite(DX_ENABLE, HIGH);  // enable sending
	encodeMessage(body);

	int counter = 0;
	_write(0xFF);
	while (data[counter] != END_MARKER)
	{
		_write(data[counter]);
		counter++;
	}
	_write(0xFF);
	_flush();

	digitalWrite(DX_ENABLE, LOW);  // disable sending
	digitalWrite(RX_ENABLE, LOW);  // enable receiving

	requestCount++; //Counts number of message sent
	return true;
}

/*
 * Read data bytes from rs485
 * Returns true if a full packet data is read and decoded, false otherwise
 */
bool Torqeedo::readMessage()//AvailableCallback fAvailable, ReadCallback fRead)
{
	if (_available())
	{
		byte input = _read();
		switch (input)
		{

		case PACKET_START:
			msgStart = true;
			len = 0;
			break;

		case PACKET_END:
			msgStart = false;
			len--;
			_checksum = data[len];
			data[len] = 0xAD;
			len = 0;
			return decodeMessage();
			break;

		default:
			if (!msgStart) //read data only if PACKET_START detected earlier
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
	}
	return false;
}


//=========================================
//            UTILITIES
//=========================================

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
	Serial3.write(content);
	return;
}
byte read2()
{
	byte input = (byte)Serial3.read();
	return input;
}
int available2()
{
	return Serial3.available();
}
void flush2()
{
	Serial3.flush();
	return;
}
