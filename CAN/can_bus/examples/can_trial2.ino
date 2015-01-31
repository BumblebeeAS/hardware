#include <can.h>
#include <SPI.h>

MCP_CAN CAN(10);
byte len = 0;
byte buf[8];

void setup()
{

   Serial.begin(115200);

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
}
unsigned char stmp[8] = {8,9,10,11,12,13,14,15};
unsigned char stmp2[8] = {3,3,3,3,3,3,3,3};
void loop()
{
	if(CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print(buf[i]);Serial.print(" ");
        }
        Serial.println();
	}
	
	//delay(50);
	CAN.sendMsgBuf(0x03, 0, 8, stmp2);
	//delay(100);
	CAN.sendMsgBuf(0x01, 0, 8, stmp);
}
