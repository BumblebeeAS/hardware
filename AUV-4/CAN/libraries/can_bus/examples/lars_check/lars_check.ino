// demo: CAN-BUS Shield, send data
#include <can.h>
#include <SPI.h>

MCP_CAN CAN(10);                                      // Set CS to pin 8

//unsigned char stmp[8] = {0, 0, 0, 10, 0, 0, 8, 2};//launch
unsigned char stmp[8] = {2, 10, 0, 0, 8, 0, 0, 0};//launch invert
//unsigned char stmp[8] = {0, 0, 0, 1, 0, 0, 0, 1};//stop
//unsigned char stmp[8] = {1, 0, 0, 0, 1, 0, 0, 0};//stop invert
//unsigned char stmp[8] = {0, 0, 0, 9, 0, 0, 10, 3};//recovery
//unsigned char stmp[8] = {3, 10, 0, 0, 9, 0, 0, 0};//recovery invert

void setup()
{
    Serial.begin(115200);
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    

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

    CAN.sendMsgBuf(0x6B, 0, 8, stmp); 
//    delay(100);           
}


void loop()
{
    // send data:  id = 0x00, standrad flame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x6B, 0, 8, stmp);
//    delay(100);                       // send data per 100ms
//Serial.println("done");
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
