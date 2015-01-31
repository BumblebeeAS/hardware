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

	CAN.init_Mask(0,0,0xFF);
	CAN.init_Mask(1,0,0xFF);
	CAN.init_Filt(0,0,0x01);                                                                       
	//CAN.init_Filt(1,0,0x03);
	CAN.init_Filt(2,0,0x03);
	//CAN.init_Filt(3,0,0x05);
	//CAN.init_Filt(4,0,0x06);
	//CAN.init_Filt(5,0,0x07);
}
unsigned char stmp[8] = {0, 1, 2, 3, 4, 5, 6, 7};

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
	CAN.sendMsgBuf(0x02, 0, 8, stmp);
	delay(100);
}
