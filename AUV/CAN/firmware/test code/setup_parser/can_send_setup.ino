#include <can.h>
#include <SPI.h>

MCP_CAN CAN(10);
uint8_t len = 0;
uint8_t buf[8];
uint32_t looptime = 0;
uint32_t looptime2 = 0;
uint8_t msg_start = 16;


void setup()
{

	Serial.begin(9600);
	pinMode(10, OUTPUT);
START_INIT:

	if (CAN_OK == CAN.begin(CAN_125KBPS) )                   // init can bus : baudrate = 500k
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
	looptime = millis();

}


//Incremented counter per msg to show number of msgs received.
uint64_t  num_array[4] = { 0, 0, 0, 0 };
uint32_t val = 12;
int32_t val2 = 3200;
uint32_t val3 = 14;
uint32_t val4 = 15;
uint32_t val5 = 0;
byte error = 0;
INT8U stmp[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

void loop()
{
	if (millis() - looptime > 500)
	{
		error = CAN.checkTXStatus(0);
		Serial.print("t0:");
		Serial.print(error);
		error = CAN.checkTXStatus(1);
		Serial.print(" t1:");
		Serial.print(error);

		error = CAN.checkError();
		Serial.print(" e:");
		Serial.print(error);
		Serial.print(" ");
		looptime = millis();
	}



	if (millis() - looptime2 > 100)
	{
		for (byte i = 0; i<4; i++)
		{
			CAN.setupCANFrame(stmp, 0, 1, val);
			CAN.setupCANFrame(stmp, 1, 2, val2);
			CAN.setupCANFrame(stmp, 3, 1, val3);
			//CAN.setupCANFrame(stmp, 3, 1, val4);
			CAN.sendMsgBuf(msg_start + i, 0, 8, stmp);
			num_array[i]++;
			
			Serial.print(msg_start + i);
			Serial.print(":");
			Serial.print(uint32_t(num_array[i]));
			Serial.print(" ");
			Serial.print(stmp[0]);
			Serial.print(" ");
			Serial.print(stmp[1]);
			Serial.print(" ");
			Serial.print(stmp[2]);
			Serial.print(" ");
			Serial.print(stmp[3]);
			Serial.print(" c ");
			val5 = CAN.parseCANFrame(stmp, 1, 2);
			Serial.print(val5);
			Serial.println();
		}
		looptime2 = millis();
	}

}
