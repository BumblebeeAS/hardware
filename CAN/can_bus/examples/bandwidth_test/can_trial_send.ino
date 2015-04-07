#include <can.h>
#include <SPI.h>

MCP_CAN CAN(10);
byte len = 0;
byte buf[8];
uint32_t looptime = 0;
uint32_t looptime2 = 0;
byte msg_start = 16;

void setup()
{

   Serial.begin(115200);
   pinMode(10,OUTPUT);
START_INIT:
    
    if(CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
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
unsigned char stmp[8] = {0, 1, 2, 3, 4, 5, 6, 7};

//Incremented counter per msg to show number of msgs received.
uint64_t  num_array[4]  = {0,0,0,0};
byte error = 0;

void loop()
{
	if(millis() - looptime > 500)
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

	

	if(millis() - looptime2 > 10)
	{
		for(byte i =0;i<4;i++)
		{
			for(byte j =0;j<8;j++)
			{
				stmp[j] = num_array[i] >> 8*j;
			}
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
			Serial.println();

		}
		looptime2 = millis();
	}
	
}
