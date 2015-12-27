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
////Telemetry Board for BBAUV 3.0
//Firmware Version :             v1.0
//
// Written by Vanessa Cassandra
// Written by Vanessa Cassandra
//
//###################################################
//###################################################
//###################################################

#include "Telemetry.h"
#include "define.h"
#include "can_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include <SD.h>	//SD card
#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>	//screen

File myFile;	//SD file
Adafruit_RA8875 tft = Adafruit_RA8875(SCREEN_CS, SCREEN_RESET);	//screen
int test;
char string[200] = "Lorem ipsum dolor sit amet, consectetur adipiscing";
char u;


MCP_CAN CAN(CAN_Chip_Select); 

unsigned char len = 0; //length of CAN message, taken care by library
unsigned char buf[8];  //Buffer for CAN message


//static means the variable is only accessible within this file
static uint8_t smartkill_buf;
static uint8_t CAN_State_Buf[3]; //CAN tx & error statues buffer
static uint8_t telemetryStats[3];
static uint8_t navStats[3]; //size to be determined
static uint8_t internalStats[10];
static uint16_t powerStats[7];
static uint8_t dummy;
static uint8_t temp;
static int16_t extPressure;

static uint32_t loopTime;
static uint32_t logTime;
static uint32_t currentTime;
static uint32_t commandTime;
static uint32_t canStatsTime;
static char presPrint[4];
static char dummyChar[9];
static String dummyStr;


int i = 0;

void setup(){
	Serial.begin(115200);
	Serial1.begin(9600); //xbee serial
	CAN_init();
	Serial.println("CAN OK");
	screen_init();
	Serial.println("Screen OK");
	currentTime = loopTime = millis();
	SD.begin(SD_CS);

	 screen_prepare();

}

int32_t test_time = 0;
void loop(){
	//send and receive messages every xx ms
	if((millis() - loopTime) > 500){
//    test_time = millis();
		screen_update();
//    Serial.println(millis() - test_time);
		// /**** WRITE INTO SD CARD ****/
		// myFile = SD.open("test.txt", FILE_WRITE);
		// myFile.write("testing 1, 2, 3.\n");
		// // close the file:
		// myFile.close();

		// tft.writeReg(0x8E, 0x80 | 0x00);	//clear screen
		// delay(30);

    loopTime = millis();
	}
  checkCANmsg();
	if((millis() - canStatsTime) > 1000){
		//check CAN status and send back status
		CAN_State_Buf[0]=CAN.checkError();
		CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
		CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
	  	Serial.println("CAN STATUS:");
		Serial.println(CAN_State_Buf[0]);
		Serial.println(CAN_State_Buf[1]);
		Serial.println(CAN_State_Buf[2]);
		CAN.sendMsgBuf(CAN_telemetry_BUS_stats, 0, 3, CAN_State_Buf); //id, extended frame(1) or normal(0), no of bytes sent, data

		canStatsTime = millis();
	}

	// /**** RECEIVE COMMAND FROM XBEE *****/
	// if((millis() - commandTime) > 200){
	// 	if (Serial1.available()){
	// 		u = Serial1.read();

	// 		if(u == 'S'){
	// 		  tft.writeReg(0x8E, 0x80 | 0x00);
	// 		}else if(u == 'L'){
	// 		  char string2[20] = "LEFT";
	// 		  tft.textSetCursor(300, 200);
	// 		  tft.textTransparent(RA8875_YELLOW);
	// 		  tft.textEnlarge(4);
	// 		  tft.textWrite(string2);
	// 		  //Serial.println("LEFT");
	// 		}else if(u == 'F'){
	// 		  char string3[20] = "FORWARD";
	// 		  tft.textSetCursor(300, 200);
	// 		  tft.textTransparent(RA8875_YELLOW);
	// 		  tft.textEnlarge(4);
	// 		  tft.textWrite(string3);
	// 		  //Serial.println("FORWARD");
	// 		}else if(u == 'B'){
	// 		  char string4[20] = "BACKWARD";
	// 		  tft.textSetCursor(300, 200);
	// 		  tft.textTransparent(RA8875_YELLOW);
	// 		  tft.textEnlarge(4);
	// 		  tft.textWrite(string4);
	// 		  //Serial.println("BACKWARD");
	// 		}else if(u == 'R'){
	// 		  char string1[20] = "RIGHT";
	// 		  tft.textSetCursor(300, 200);
	// 		  tft.textTransparent(RA8875_YELLOW);
	// 		  tft.textEnlarge(4);
	// 		  tft.textWrite(string1);
	// 		  //Serial.println("RIGHT");
	// 		}
	// 	}
	// 	commandTime = millis();
	// }

}


void screen_init(){

	/* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
	if (!tft.begin(RA8875_800x480)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	tft.displayOn(true);
	tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
	tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	tft.PWM1out(255);
	tft.fillScreen(RA8875_GREEN);	//Fill screen with cyan color
  	delay(100);
	tft.fillScreen(RA8875_CYAN);
  	delay(100);
	tft.fillScreen(RA8875_BLUE);
  	delay(100);
	tft.fillScreen(RA8875_RED);
  	delay(100);
	tft.fillScreen(RA8875_RED);
	delay(100);
	tft.fillScreen(RA8875_MAGENTA);
	delay(100);
	tft.fillScreen(RA8875_YELLOW);
	delay(100);
	tft.fillScreen(RA8875_BLACK);

	/* Switch to text mode */  
	tft.textMode();

	/* Change the cursor location and color ... */  
	tft.textSetCursor(0, 12);
	tft.textEnlarge(4);
	tft.textTransparent(RA8875_YELLOW);

	/* ... and render some more text! */
	tft.textWrite("Hello");
}

void screen_prepare(){
	  tft.fillScreen(RA8875_BLACK);
	  tft.textEnlarge(1);
  	tft.textTransparent(RA8875_WHITE);
    tft.textSetCursor(0, 0);
    tft.textWrite("Ext press:");
    tft.textSetCursor(0, 40);
  	tft.textWrite("Int press:");
  	tft.textSetCursor(0, 80);
  	tft.textWrite("Humidity:");
    tft.textSetCursor(0, 120);
    tft.textWrite("PMB1 press:");
    tft.textSetCursor(0, 160);
    tft.textWrite("PMB2 press:");
    tft.textSetCursor(0, 200);
    tft.textWrite("PMB1 temp:");
    tft.textSetCursor(0, 240);
    tft.textWrite("PMB2 temp:");
    tft.textSetCursor(0, 280);
    tft.textWrite("Backplane temp:");
    tft.textSetCursor(0, 320);
    tft.textWrite("SA temp:");
    tft.textSetCursor(0, 360);
    tft.textWrite("Thruster temp1:");
    tft.textSetCursor(0, 400);
    tft.textWrite("Thruster temp2:");
    
    tft.textSetCursor(400, 0);
    tft.textWrite("Batt1 capacity:");
    tft.textSetCursor(400, 40);
    tft.textWrite("Batt2 capacity:");
    tft.textSetCursor(400, 80);
    tft.textWrite("Batt1 current:");
    tft.textSetCursor(400, 120);
    tft.textWrite("Batt2 current:");
    tft.textSetCursor(400, 160);
    tft.textWrite("Batt1 voltage:");
    tft.textSetCursor(400, 200);
    tft.textWrite("Batt2 voltage:");

  	delay(10);
}

void screen_update(){
  screen_write(160, 0, extPressure);
	screen_write(160, 40, internalStats[INT_PRESS]);
	screen_write(150, 80, internalStats[HUMIDITY]);
	screen_write(177, 120, internalStats[PMB1_PRESS]);
	screen_write(177, 160, internalStats[PMB2_PRESS]);
	screen_write(168, 200, internalStats[PMB1_TEMP]);
	screen_write(168, 240, internalStats[PMB2_TEMP]);
	screen_write(250, 280, internalStats[BP_TEMP]);
	screen_write(140, 320, internalStats[SA_TEMP]);
	screen_write(250, 360, internalStats[TH_TEMP1]);
	screen_write(250, 400, internalStats[TH_TEMP2]);

	screen_write(645, 0, powerStats[BATT1_CAPACITY]);
	screen_write(645, 40, powerStats[BATT2_CAPACITY]);
	screen_write(635, 80, powerStats[BATT1_CURRENT]);
	screen_write(635, 120, powerStats[BATT2_CURRENT]);
	screen_write(635, 160, powerStats[BATT1_VOLTAGE]);
	screen_write(635, 200, powerStats[BATT2_VOLTAGE]);

	delay(50);
}

void screen_write(uint32_t x, uint32_t y, uint8_t var){
	tft.fillRect(x, y, 50, 30, RA8875_BLACK);
	tft.textTransparent(RA8875_YELLOW);
	tft.textSetCursor(x, y);
	dummyStr = String(var);
	dummyStr.toCharArray(dummyChar, 9);
	tft.textWrite(dummyChar);
}

void CAN_init(){
START_INIT:
    if(CAN_OK == CAN.begin(CAN_1000KBPS)){                   // init can bus : baudrate = 500k
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init ok!");
        #endif           
    }
    else{
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init fail");
          Serial.println("Init CAN again");
          delay(1000);
        #endif           
        goto START_INIT;
    }
}

void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		switch(CAN.getCanId()){
			case CAN_pressure:
				// pressure = buf[1];	//get first 8 bit
				// pressure = pressure << 8;	//shift left
				// pressure = pressure | buf[0];
				Serial.println("Pressure");
				extPressure = CAN.parseCANFrame(buf, 0, 2);
				break;

			case 6:
				Serial.println("CAN SA");
				internalStats[SA_TEMP] = CAN.parseCANFrame(buf, 0, 1);
				internalStats[HUMIDITY] = CAN.parseCANFrame(buf, 1, 1);
				internalStats[INT_PRESS] = CAN.parseCANFrame(buf, 2, 1);
				break;

			case CAN_thruster_stats:
				internalStats[TH_TEMP1] = CAN.parseCANFrame(buf, 0, 1);
				internalStats[TH_TEMP2] = CAN.parseCANFrame(buf, 1, 1);
				break;

			case CAN_PMB1_stats2:
				powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 4, 2);
				powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 6, 2);
				break;

			case CAN_PMB1_stats3:
				powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
				internalStats[PMB1_TEMP] = CAN.parseCANFrame(buf, 1, 1);
				internalStats[PMB1_PRESS] = CAN.parseCANFrame(buf, 2, 1);
				break;

			case CAN_PMB2_stats2:
				powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 4, 2);
				powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 4, 2);
				break;

			case CAN_PMB2_stats3:
				powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
				internalStats[PMB2_TEMP] = CAN.parseCANFrame(buf, 1, 1);
				internalStats[PMB2_PRESS] = CAN.parseCANFrame(buf, 2, 1);
				break;

			case CAN_backplane_stats:
				internalStats[BP_TEMP] = CAN.parseCANFrame(buf, 0, 1);
				break;

			case CAN_backplane_stats2:
				break;

			case CAN_DVL_velocity:

				break;

			case CAN_earth_odometry:
				break;

			case CAN_relative_odometry:
				break;

			case CAN_imu_1:
				break;
			
			case CAN_imu_2:
				break;

			default:
				break;
		}

		CAN.clearMsg();
	}
}
