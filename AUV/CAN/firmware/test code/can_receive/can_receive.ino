
#include <can.h>
#include <SPI.h>


unsigned char Flag_Recv = 0;
unsigned char len = 0;
unsigned char buf[8];
char str[20];

unsigned long int total_time = 0;
unsigned long int total_byte = 0;
float val = 0;
MCP_CAN CAN(8);                                           

void setup()
{
	Serial.begin(115200);

START_INIT:

	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS Shield init ok!");
		Serial.println("CAN receiver...");
	}
	else
	{
		Serial.println("CAN BUS Shield init fail");
		Serial.println("Init CAN BUS Shield again");
		delay(100);
		goto START_INIT;
	}
}


void loop()
{
	if (CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
	{
		// Serial.println("t");
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		for (int i = 0; i<len; i++)    // print the data
		{
			Serial.print(buf[i]); Serial.print("\t");
			total_byte++;
		}
		Serial.println();
	}

	total_time = millis();
	//Serial.print("br:");
	val = float((float)total_byte * 1000 / (float)total_time);
	Serial.println(val * 8);
	delay(100);
	/*
	if(total_time > 60000)
	{
	total_time = 0;
	total_byte = 0;
	}*/
}

/*********************************************************************************************************
END FILE
*********************************************************************************************************/
