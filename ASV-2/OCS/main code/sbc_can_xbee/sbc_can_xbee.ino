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

#include <can.h>
#include <SPI.h>
#include <XBee.h>
#include "can_asv_defines.h"
#include "defines.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
uint32_t screen_loop = 0;

uint32_t ocs_heartbeat = 0;
uint32_t asv_heartbeat = 0;

//Xbee setup
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B9783); //Telemetry
//XBeeAddress64 addr64spare = XBeeAddress64(0x0013A200, 0x416B96A1); //Telemetry
XBeeAddress64 addr64spare = XBeeAddress64(0x0013A200, 0x416B96A3); //Spare1
ZBTxRequest zbTx;
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

//Oled setup
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup()
{
	/* add setup code here */
	Serial.begin(115200);
	Serial1.begin(115200);
	xbee.begin(Serial1);
	sbc_bus_loop = millis();
	heartbeat_loop = millis();
	screen_loop = millis();

	ocs_heartbeat = millis();
	asv_heartbeat = millis();

	Serial.println("INITIATING SCREEN...");
	display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	//while (1);
	Serial.println("INITIATING TRANSMISSION...");

}

uint8_t led_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t cur_time = 0;

//Serial read frame
int16_t incoming_data = 0;
uint8_t read_flag = 0;
uint8_t read_buffer[13];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;

bool blink = true;

void loop()
{
	/*****************************************/
	/*		Screen							 */
	/*****************************************/
	if ((millis() - screen_loop) > 1000)
	{
		display.clearDisplay();
		display.setCursor(0, 0);
		blink = !blink;
		display.print(blink ? "*- " : "-* ");
		display.print("From OCS: ");
		display_heartbeat(ocs_heartbeat);
		display.print(blink ? "-* " : "*- ");
		display.print("From ASV: ");
		display_heartbeat(asv_heartbeat);
		display.print("ctr:");		
		display.print(read_ctr);
		display.print(" s:");
		display.println(read_size);
		display.display();
		//Serial.println("ALIVE");

		screen_loop = millis();
	}

	/*
	/*****************************************/
	/*  Heartbeat							 */
	/*  Maintain comms with SBC				 */
	/*****************************************/

	if ((millis() - heartbeat_loop) > HEARTBEAT_TIMEOUT)
	{
		//CAN.setupCANFrame(buf, 0, 0, HEARTBEAT_OCS);
		//buf[0] = HEARTBEAT_OCS;
		//CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
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
	/*  Transmit commands to XBee			 */
	/*****************************************/
	if (Serial.available())
	{
		ocs_heartbeat = millis();
		//read
		while (incoming_data > -1)
		{
			incoming_data = Serial.read();
			/*
			Serial.print(read_ctr);
			Serial.print("s");
			Serial.print(read_size);
			Serial.print("d");
			Serial.print((uint8_t)incoming_data, HEX);
			Serial.print("| ");
			//Serial.write(incoming_data);
			//display.print(read_ctr);
			//display.print((uint8_t)incoming_data, HEX);
			//display.display();*/
			if (incoming_data == -1)
			{
				incoming_data = 0;
				break;
			}
			if (incoming_data == START_BYTE && !read_flag)
			{
				//Serial.print("[1]");
				read_buffer[read_ctr] = START_BYTE;
				read_flag = 1;
				read_ctr = 1;
			}
			else if (incoming_data == START_BYTE && read_flag == 1)
			{
				//Serial.print("[2]");
				read_buffer[read_ctr] = START_BYTE;
				read_flag++;
			}
			else if (read_flag == 2)
			{
				//Serial.print("[D]");
				if (read_ctr == 1)
				{
					read_id = incoming_data;
					read_buffer[2] = read_id;
					read_ctr++;
				}
				else if (read_ctr == 2)
				{
					read_size = incoming_data;
					read_buffer[3] = read_size;
					read_ctr++;
					// size must be between 1-8 bytes
					if ((read_size > 8) || (read_size == 0))
					{
						//Serial.println(" REJ");
						read_flag = 0;
						read_ctr = 0;
					}
				}
				else if (read_ctr > 12)
				{
					//Serial.println(" REJ");
					read_flag = 0;
					read_ctr = 0;
				}
				else if (read_ctr > 2)
				{
					read_buffer[read_ctr + 1] = incoming_data;
					if (read_ctr == (2 + read_size))
					{
						//Check for HEARTBEAT from SBC
						if (read_id == CAN_heartbeat)
						{
							//Serial.println("hb!");
						}
						//Serial.println(" SEND");
						sendXbeeMsg();
						read_flag = 0;
						read_ctr = 0;
					}
					else {
						//read_buffer[read_ctr - 3] = incoming_data;
						read_ctr++;
					}
				}

			}
		}
	}

	/**********************************************/
	/*Check for incoming CAN messages from Xbee in CAN Format*/
	/*			 Transmit CAN messages to SBC     */
	/**********************************************/

	xbee.readPacket();

	if (xbee.getResponse().isAvailable()) {
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			asv_heartbeat = millis();
			// got a zb rx packet
			// now fill our zb rx class
			xbee.getResponse().getZBRxResponse(rx);
			uint8_t len = rx.getDataLength();
			for (int i = 0; i < len; i++)	Serial.write(rx.getData(i));
		}
	}
}

//==========================================
//          OLED FUNCTIONS
//==========================================

void display_heartbeat(uint32_t heartbeat)
{
	if ((millis() - heartbeat) > 2000)
	{
		display.println("NO ");
	}
	else
	{
		display.println("YES");
	}
}

//==========================================
//          XBEE FUNCTIONS
//==========================================

uint8_t getRssi() {
	xbee.send(atRequest);

	// wait up to 5 seconds for the status response
	if (xbee.readPacket(100)) {
		// got a response!

		// should be an AT command response
		if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			xbee.getResponse().getAtCommandResponse(atResponse);

			if (atResponse.isOk()) {
				return atResponse.getValue()[0];
			}
		}
	}
}
#define _XBEE_DEBUG_
#ifndef _XBEE_DEBUG_
void sendXbeeMsg()
{
	zbTx = ZBTxRequest(addr64, read_buffer, read_size + 4);
	xbee.send(zbTx);
	if (xbee.readPacket(100))
	{
		// got a response!
		// should be a znet tx status      

		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
		{
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			// get the delivery status, the fifth byte
			if (txStatus.getDeliveryStatus() == SUCCESS)
			{
				// success.  time to celebrate
				Serial.println("Ack");
			}
			else
			{

				Serial.println("No Acknowledgement");
			}
		}
		else
		{
			// local XBee did not provide a timely TX Status Response -- should not happen
			Serial.println("Sender Error");
		}
	}
}
#else

void sendXbeeMsg()
{
	sendXbeeMsgtoAddr(addr64);
	//sendXbeeMsgtoAddr(addr64spare);
}
void sendXbeeMsgtoAddr(XBeeAddress64 addr)
{
	zbTx = ZBTxRequest(addr, read_buffer, read_size + 4);
	xbee.send(zbTx);
	//Serial.print(" XbeeSend ");
	//display.print("SEND~ ");
	if (xbee.readPacket(100))
	{
		// got a response!
		// should be a znet tx status      

		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
		{
			xbee.getResponse().getZBTxStatusResponse(txStatus);
			// get the delivery status, the fifth byte
			if (txStatus.getDeliveryStatus() == SUCCESS)
			{
				// success.  time to celebrate
				//Serial.println("Ack");
				//display.print(" O ");
			}
			els
			{

				//Serial.println("No Acknowledgement");
				//display.print(" X ");
			}
		}
	}
	else
	{
		// local XBee did not provide a timely TX Status Response -- should not happen
		//Serial.println("Sender Error");
		//display.print(" - ");
	}
	//display.display();

}
#endif