#include <can.h>
#include <SPI.h>

MCP_CAN CAN(10);
byte len = 0;
byte buf[8];
uint32_t num_array[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void setup()
{

	Serial.begin(115200);

START_INIT:

	if (CAN_OK == CAN.begin(CAN_125KBPS))                   // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS Shield init ok!");
	}
	else
	{
		Serial.println("CAN BUS Shield init fail");
		Serial.println("Init CAN BUS Shield again");
		delay(100);
		goto START_INIT;
	}
}
unsigned char stmp[8] = { 8, 9, 10, 11, 12, 13, 14, 15 };
unsigned char stmp2[8] = { 3, 3, 3, 3, 3, 3, 3, 3 };
byte error = 0;
unsigned long id = 0;
void loop()
{
	uint32_t val = 0;
	/*
	error = CAN.checkError();
	Serial.print("e:");
	Serial.print(error);
	Serial.print(" ");
	*/
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		for (int i = len - 1; i>-1; i--)    // print the data
		{
			val = val + buf[i];
			if (i > 0)	val = val << 8;
		}
		Serial.print(id);
		Serial.print(":");
		Serial.print(val);
		//Serial.print(",");
		//Serial.print(val - num_array[id]);
		Serial.println();
		num_array[id]++;
		//Serial.println();
	}
}
