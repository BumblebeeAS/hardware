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
//Firmware Version :             v1.1
//
// Written by Goh Eng Wei
// Change Log for v1.1:
// - Added heartbeat to sbc CAN
// - Removed CAN status messsages to be sent out. Redundant.
//###################################################
//###################################################
//###################################################

#include "can.h"
#include <SPI.h>
#include "can_asv3_defines.h"
#include "can_asv_defines.h"
#include "defines.h"

MCP_CAN CAN(8);
uint8_t buf[8];
uint32_t id = 0;
uint8_t len = 0;

uint8_t heartbeat;
uint8_t sbc_bus_stats[3];
uint16_t heartbeat_ctr = 0;
uint32_t heartbeat_loop = 0;
uint32_t sbc_bus_loop = 0;
uint32_t time_test = 0;

void setup()
{
	/* add setup code here */
	Serial.begin(115200);
	Serial.println("Hi this is SBC-CAN");
	sbc_bus_loop = millis();
	heartbeat_loop = millis();
START_INIT:

	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
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
	Serial.println("INITIATING TRANSMISSION...");
}

uint8_t led_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;

//Serial read frame
int16_t incoming_data = 0;  
uint8_t read_flag = 0;
uint8_t read_buffer[8];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;

void loop()
{
	/*
	/*****************************************/
	/*  Heartbeat							 */
	/*  SBC-CAN Heartbeat    				 */
	/*****************************************/

	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		CAN.setupCANFrame(buf, 0, 0, CAN_HEARTBEAT);
		buf[0] = 0x09; // Define in the google drive for Logic Backplane heartbeat.
		CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
		heartbeat_loop = millis();
	}

	/*****************************************/
	/*  Transmit CAN Diagnostics			 */
	/*****************************************/

	if ((millis() - sbc_bus_loop) > 1000)
	{
		//CAN.setupCANFrame(sbc_bus_stats, 0, 1, CAN.checkError());
		//CAN.setupCANFrame(sbc_bus_stats, 1, 1, CAN.checkTXStatus(0));
		//CAN.setupCANFrame(sbc_bus_stats, 2, 1, CAN.checkTXStatus(1));
		//CAN.sendMsgBuf(CAN_SBC_BUS_stats, 0, 4, sbc_bus_stats);

		sbc_bus_loop = millis();
	}

	/*****************************************/
	/*  Check for incoming commands from SBC */
	/*  Transmit commands to CAN Bus		 */
	/*****************************************/

	if (Serial.available())
	{
		//read
		while (incoming_data > -1)
		{
			incoming_data = Serial.read();
			if (incoming_data == -1)
			{
				incoming_data = 0;
				break;
			}
			if (incoming_data == START_BYTE && !read_flag)
			{
				read_flag = 1;
				read_ctr = 1;
			}
			else if (incoming_data == START_BYTE && read_flag == 1)
			{
				read_flag++;
			}
			else if (read_flag == 2)
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
					read_buffer[read_ctr - 3] = incoming_data;
					if (read_ctr == (2 + read_size))
					{
						//Check for HEARTBEAT from SBC
						if (read_id == CAN_HEARTBEAT)
						{
							//Serial.println("hb!");
						}
						CAN.sendMsgBuf(read_id, 0, read_size, read_buffer);
						read_flag = 0;
						read_ctr = 0;
					}
					else {
						read_buffer[read_ctr - 3] = incoming_data;
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

    switch (id) {
      case 1541:
      case 1542:
      case 1797:
      case 1798:
        break;
      default:
        
        Serial.write(START_BYTE);
        Serial.write(START_BYTE);
        Serial.write(id);
        Serial.write(len);
        for (int i = 0; i < len; i++) Serial.write(buf[i]); 
        
        break;
        
    }
    /*
    Serial.println("Im here");
    if (true) {
	    Serial.print(START_BYTE);
	    Serial.print(START_BYTE);
	    Serial.print(id);
	    Serial.print(len);
	    for (int i = 0; i < len; i++)	Serial.print(buf[i]);
    }
    */
	}
}
