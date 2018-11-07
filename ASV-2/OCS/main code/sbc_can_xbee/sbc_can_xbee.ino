#include <can.h>
#include <SPI.h>
#include "can_asv_defines.h"
#include "defines.h"
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

uint8_t heartbeat;
uint8_t sbc_bus_stats[3];
uint16_t heartbeat_ctr = 0;
uint32_t heartbeat_loop = 0;
uint32_t sbc_bus_loop = 0;
uint32_t time_test = 0;
uint32_t screen_loop = 0;

uint32_t ocs_heartbeat = 0;
uint32_t asv_heartbeat = 0;

//Oled setup
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup()
{
	/* add setup code here */
	Serial.begin(115200);
	Serial1.begin(9600);
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

//Serial buffer from odroid
int16_t incoming_data = 0;
uint8_t read_flag = 0;
uint8_t read_buffer[13];
uint8_t read_size;
uint8_t read_id;
uint8_t read_ctr;

// Serial buffer from radio
int16_t radio_incoming_data = 0;
uint8_t radio_read_flag = 0;
uint8_t radio_read_buffer[11]; // Takes [id,len,data,crc] no FE FE
uint8_t radio_read_size;
uint8_t radio_read_id;
uint8_t radio_read_ctr; // Counts size of incoming_data without FE FE
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
	
	/********************************************/
	/* Check for incoming commands from Odroid	*/
	/*  Transmit commands to Radio				*/
	/********************************************/
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
						//Serial.println(" SEND");
						forwardToRadio(read_id, read_size, read_buffer);
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

	/******************************************************/
	/*Check for incoming messages from Radio in CAN Format*/
	/*			 Transmit Radio messages to Odroid		  */
	/******************************************************/

	// RECEIVE
	if (Serial1.available() > 0)
	{
		asv_heartbeat = millis();
		//read
		while (radio_incoming_data > -1)
		{
			radio_incoming_data = Serial1.read();
			if (radio_incoming_data == -1)
			{
				radio_incoming_data = 0;
				break;
			}
			if (radio_incoming_data == START_BYTE && !radio_read_flag)
			{
				radio_read_flag = 1;
			}
			else if (radio_incoming_data == START_BYTE && radio_read_flag == 1)
			{
				radio_read_flag++;
				radio_read_flag = 2;
			}
			else if (radio_read_flag == 2)
			{
				if (radio_read_ctr == 0)
				{
					radio_read_id = radio_incoming_data;
					radio_read_buffer[radio_read_ctr] = radio_incoming_data;
					radio_read_ctr++;
				}
				else if (radio_read_ctr == 1)
				{
					radio_read_size = radio_incoming_data;
					radio_read_buffer[radio_read_ctr] = radio_incoming_data;
					radio_read_ctr++;
				}
				else if (radio_read_ctr >= 2)
				{
					radio_read_buffer[radio_read_ctr] = radio_incoming_data;
					if (radio_read_ctr == (2 + radio_read_size))
					{
						// Forward to ocs only if crc is correct
						if (isValidCrc(radio_read_buffer, radio_read_ctr))
						{
							// Full packet received
							forwardToOcs(radio_read_size, radio_read_buffer);
							// Print out received msg
//							Serial.print("id: ");
//							Serial.print(radio_read_id);
//							Serial.print(" size: ");
//							Serial.print(radio_read_size);
//							Serial.print(" data: ");
//							for (int i = 2; i < radio_read_ctr; i++)
//							{
//								Serial.print(radio_read_buffer[i]);
//								Serial.print(" ");
//							}
//							Serial.println("");
						}
						radio_read_flag = 0;
						radio_read_ctr = 0;
					}
					else {
						radio_read_buffer[radio_read_ctr] = radio_incoming_data;
						radio_read_ctr++;
						//Serial.print("|");
					}
				}
			}
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
//          RADIO FUNCTIONS
//==========================================

// data[] is [id, len, data]
void forwardToOcs(int len, uint8_t data[]) {
	uint8_t temp[13] = { 0 };

	temp[0] = 0xFE;
	temp[1] = 0xFE;
	for (int i = 2, j = 0; j < len+2; i++, j++) {
		temp[i] = data[j];
	}

	for (int i = 0; i<len + 4; i++) {
		//Serial.write(temp[i]);
		//Serial.print(temp[i],HEX);
		//Serial.print(" ");
	}
	Serial.flush();
	//Serial.println();
	//Serial.print("SEND");
}

// data[] is only data
void forwardToRadio(int id, int len, uint8_t data[]) {
	uint8_t temp[13] = { 0 };

	temp[0] = 0xFE;
	temp[1] = 0xFE;
	temp[2] = id;
	temp[3] = len;
	for (int i = 2, j = 0; j < len; i++, j++) {
		temp[i] = data[j];
	}
	// Take checksum of [id,len,data] in case [id] and [len] have errors too
	uint8_t crc = checksum(temp + 2, len + 2);
	temp[len + 4] = crc;

	for (int i = 0; i<len + 5; i++) {
		Serial1.write(temp[i]);
		//Serial.print(temp[i],HEX);
		//Serial.print(" ");
	}
	Serial1.flush();
	//Serial.println();
	//Serial.print("SEND");
}

// Takes the crc of [len] bytes in [data] and compares to last byte
// data[] is [id,len,data] without FE FE
bool isValidCrc(uint8_t data[], int len) {
	uint8_t crc = checksum(data, len);
	if (crc == data[len]) {
		return true;
	}
	return false;
}

uint8_t checksum(uint8_t data[], uint8_t len) {
	uint8_t crc = 0;
	uint8_t i = 0;
	while (len != 0) {
		crc = crc ^ data[i];
		i++;
		len--;
	}
	return crc;
}
