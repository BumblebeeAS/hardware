//###################################################
//###################################################
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
////SBC to CAN interface for BBAUV 3.0
//Firmware Version :             v1.0
//
// Written by Goh Eng Wei
// Change Log for v1.0:
//###################################################
//###################################################
//###################################################

#include <can.h>
#include <SPI.h>
#include "can_defines.h"
#include "defines.h"

MCP_CAN CAN(8);
uint8_t buf[8];
uint32_t id = 0;
uint8_t len = 0;
uint8_t sbc_bus_stats[3];
uint16_t heartbeat_ctr = 0;
uint16_t heartbeat_loop = 0;
uint16_t sbc_bus_loop = 0;

void setup()
{
  /* add setup code here */
	Serial.begin(115200);

START_INIT:

	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS: OK");
	}
	else
	{
		Serial.println("CAN BUS: FAILED");
		Serial.println("CAN BUS: Reinitializing");
		delay(1000);
		goto START_INIT;
	}
}

uint8_t led_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;

//Serial read frame
uint8_t incoming_data = 0;
uint8_t read_flag = 0;
uint8_t read_buffer[8];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;
void loop()
{
	/*****************************************/
	/*  Heartbeat							 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

	if (heartbeat_loop - millis() > HEARTBEAT_TIMEOUT)
	{
		//dosomething
	}

	/*****************************************/
	/*  Transmit CAN Diagnostics			 */
	/*****************************************/

	if (sbc_bus_loop - millis() > 1000)
	{
		CAN.setupCANFrame(sbc_bus_stats, 0, 1, CAN.checkError());
		CAN.setupCANFrame(sbc_bus_stats, 1, 1, CAN.checkTXStatus(0));
		CAN.setupCANFrame(sbc_bus_stats, 2, 1, CAN.checkTXStatus(1));
		CAN.sendMsgBuf(CAN_SBC_BUS_stats, 0, 3, sbc_bus_stats);
		sbc_bus_loop = millis();
	}

	/*****************************************/
	/*  Check for incoming commands from SBC */
	/*  Transmit commands to CAN Bus		 */
	/*****************************************/

	if (Serial.available())
	{
		//read
		while(incoming_data = Serial.read())
		{
			if (incoming_data == START_BYTE && !read_flag)
			{
				read_flag = 1;
				read_ctr = 1;
			}
			else if (read_flag)
			{
				if (read_ctr == 1)
				{
					read_id = incoming_data;
					read_ctr++;
				}
				else if (read_ctr == 2)
				{
					read_size = incoming_data;
					read_ctr++;
				}
				else if (read_ctr > 2)
				{
					if (read_ctr = 2 + read_size)
					{
						//Check for HEARTBEAT from SBC
						if (read_id == CAN_HEARBEAT)
						{
							heartbeat_loop = millis();
						}
						//CAN.sendMsgBuf(read_id, 0, read_size, read_buffer);
						read_flag = 0;
						read_ctr = 0;
					}
					else {
						read_buffer[read_ctr - 2] = incoming_data;
						read_ctr++;
					}
				}

			}
		}
	}

	/**********************************************/
	/*Check for incoming CAN messages from CAN Bus*/
	/*			 Transmit CAN messages to SBC     */
	/**********************************************/
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		CAN.readMsgBufID(&id, &len, buf);// read data,  len: data length, buf: data buf
		Serial.write(START_BYTE);
		Serial.write(id);
		Serial.write(len);
		Serial.write(buf, len);
	}
}

/*
void byteStuff(uint8_t &buffer)
{
	for (int i = 0; i < 8; i++)

	{
		if (buffer[i] == START_BYTE)
		{
			
		}
	}
}
*/