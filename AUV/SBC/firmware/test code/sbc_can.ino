#include <can.h>
#include <SPI.h>
#include "can_defines.h"

MCP_CAN CAN(8);
byte buf[8];
uint16_t pres = 0;
uint32_t id = 0;
uint8_t len = 0;
uint8_t humidity, temp, int_pres;
uint8_t sa_bus_stats[3];
void setup()
{
  /* add setup code here */
	Serial.begin(115200);

START_INIT:

	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS init ok!");
	}
	else
	{
		Serial.println("CAN BUS init fail");
		Serial.println("Init CAN BUS again");
		delay(1000);
		goto START_INIT;
	}
}

uint8_t led_buf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;
void loop()
{
  /* add main program code here */
	//Serial.println("reading...");
	if (Serial.available())
	{
		//read
	}

	if (cur_time - millis() > 500)
	{
		CAN.sendMsgBuf(CAN_LED, 0,8, led_buf);
		if(led_buf[0] <9) led_buf[0]++;
		else led_buf[0] = 0;
		if (led_buf[1] <9) led_buf[1]++;
		else led_buf[1] = 0;
		cur_time = millis();
	}

	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBufID(&id, &len, buf);// read data,  len: data length, buf: data buf
		switch (id){
		case CAN_pressure:
			pres = CAN.parseCANFrame(buf, 0, 2);
			Serial.print("pres:");
			Serial.println(pres);
			break;
		case CAN_SA_stats:
			temp = CAN.parseCANFrame(buf, 0, 1);
			int_pres = CAN.parseCANFrame(buf, 1, 1);
			humidity = CAN.parseCANFrame(buf, 2, 1);
			Serial.print("int stats:");
			Serial.print(temp);
			Serial.print(" ");
			Serial.print(int_pres);
			Serial.print(" ");
			Serial.print(humidity);
			Serial.println();
			break;

		case CAN_SA_BUS_stats:
			sa_bus_stats[0] = CAN.parseCANFrame(buf, 0, 1);
			sa_bus_stats[1] = CAN.parseCANFrame(buf, 1, 1);
			sa_bus_stats[2] = CAN.parseCANFrame(buf, 2, 1);
			Serial.print("bus stats:");
			Serial.print(sa_bus_stats[0]);
			Serial.print(" ");
			Serial.print(sa_bus_stats[1]);
			Serial.print(" ");
			Serial.print(sa_bus_stats[2]);
			Serial.println();
			break;
		}
	}
	//delay(50);


}
