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

//Xbee setup
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B9783); //Telemetry
//XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B96A3); //Spare
ZBTxRequest zbTx;
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

void setup()
{
  /* add setup code here */
	Serial.begin(115200);
	Serial1.begin(115200);
	xbee.begin(Serial1);
	sbc_bus_loop = millis();
	heartbeat_loop = millis();
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

void loop()
{
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
	
	
	if ((millis() - sbc_bus_loop ) > 1000)
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
		//read
		while(incoming_data > -1)
		{
			incoming_data = Serial.read();
			if (incoming_data == -1)
			{
				incoming_data = 0;
				break;
			}
			if (incoming_data == START_BYTE && !read_flag)
			{
				read_buffer[read_ctr] = START_BYTE;
				read_flag = 1;
				read_ctr = 1;
			}
			else if (incoming_data == START_BYTE && read_flag == 1)
			{
				read_buffer[read_ctr] = START_BYTE;
				read_flag++;
			}
			else if (read_flag == 2)
			{
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
			// got a zb rx packet
			// now fill our zb rx class
			xbee.getResponse().getZBRxResponse(rx);
			uint8_t len = rx.getDataLength();
			for(int i = 0;i<len;i++)	Serial.write(rx.getData(i));
		}
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

void sendXbeeMsg()
{
	zbTx = ZBTxRequest(addr64, read_buffer,read_size + 4);
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