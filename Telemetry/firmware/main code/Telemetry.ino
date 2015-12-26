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
static uint8_t navStats[3]; //size to be determined
static uint8_t internalStats[9];
static uint8_t powerStats[7];
static uint8_t dummy;
static uint8_t temp;

static uint32_t loopTime;
static uint32_t logTime;
static uint32_t currentTime;
static uint32_t commandTime;

void setup(){
	Serial.begin(115200);
	Serial1.begin(9600); //xbee serial

	CAN_init();
	Serial.println("CAN OK");

	/* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
	if (!tft.begin(RA8875_800x480)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	tft.displayOn(true);
	tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
	tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	tft.PWM1out(255);
	tft.fillScreen(RA8875_CYAN);	//Fill screen with cyan color

	/* Switch to text mode */  
	tft.textMode();

	/* Change the cursor location and color ... */  
	tft.textSetCursor(100, 100);
	tft.textTransparent(RA8875_YELLOW);

	/* ... and render some more text! */
	tft.textWrite(string);

	currentTime = loopTime = millis();

	SD.begin(SD_CS);
  


}

void loop(){
	//send and receive messages every xx ms
	currentTime = millis();

	if(currentTime > loopTime + 200){
		/**** WRITE INTO SD CARD ****/
		myFile = SD.open("test.txt", FILE_WRITE);
		myFile.write("testing 1, 2, 3.\n");
		// close the file:
		myFile.close();


		/**** SHOW STUFF ON SCREEN *****/
		tft.textMode();

		if (test > 480){
			test = 0;
			tft.writeReg(0x8E, 0x80 | 0x00);
			delay(30);
		}

		checkCANmsg();

	//    Serial.println(StatsBuf[0]);
	//    Serial.println(StatsBuf[1]);
	    CAN.sendMsgBuf(CAN_backplane_stats, 0, 1, internalStats); //id, extended frame(1) or normal(0), no of bytes sent, data
    
    
//		StatsBuf[0] = currentReading[0];
//		StatsBuf[1] = currentReading[1];
//		StatsBuf[2] = voltageReading5V;
//		StatsBuf[3] = voltageReading12V;
//		StatsBuf2[0] = dummy;


//		//Publish stats onto the CAN bus
//		CAN.sendMsgBuf(CAN_backplane_stats, 0, 3, StatsBuf);
//		Serial.println(StatsBuf[0]);
//		Serial.println(StatsBuf[1]);
//		Serial.println(StatsBuf[2]);

		//check CAN status and send back status
		CAN_State_Buf[0]=CAN.checkError();
		CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
		CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
	  Serial.println("CAN STATUS:");
		Serial.println(CAN_State_Buf[0]);
		Serial.println(CAN_State_Buf[1]);
		Serial.println(CAN_State_Buf[2]);
		CAN.sendMsgBuf(CAN_backplane_BUS_stats, 0, 3, CAN_State_Buf); //CAN_backplane_BUS_stats ID = 22

		loopTime = currentTime;

	}

	/**** RECEIVE COMMAND FROM XBEE *****/
	currentTime = millis();
	if(currentTime > commandTime + 50){
		if (Serial1.available()){
			u = Serial1.read();

			if(u == 'S'){
			  tft.writeReg(0x8E, 0x80 | 0x00);
			}else if(u == 'L'){
			  char string2[20] = "LEFT";
			  tft.textSetCursor(300, 200);
			  tft.textTransparent(RA8875_YELLOW);
			  tft.textEnlarge(4);
			  tft.textWrite(string2);
			  //Serial.println("LEFT");
			}else if(u == 'F'){
			  char string3[20] = "FORWARD";
			  tft.textSetCursor(300, 200);
			  tft.textTransparent(RA8875_YELLOW);
			  tft.textEnlarge(4);
			  tft.textWrite(string3);
			  //Serial.println("FORWARD");
			}else if(u == 'B'){
			  char string4[20] = "BACKWARD";
			  tft.textSetCursor(300, 200);
			  tft.textTransparent(RA8875_YELLOW);
			  tft.textEnlarge(4);
			  tft.textWrite(string4);
			  //Serial.println("BACKWARD");
			}else if(u == 'R'){
			  char string1[20] = "RIGHT";
			  tft.textSetCursor(300, 200);
			  tft.textTransparent(RA8875_YELLOW);
			  tft.textEnlarge(4);
			  tft.textWrite(string1);
			  //Serial.println("RIGHT");
			}
		}
		commandTime = currentTime;
	}

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

    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
    
    //register number, extension, 
	CAN.init_Filt(0, 0, CAN_backplane_kill);	//smart kill
	CAN.init_Filt(1, 0, CAN_backplane_stats);	//Backplane statistics
	CAN.init_Filt(2, 0, CAN_backplane_stats2);	//Backplane statistics 2
	CAN.init_Filt(3, 0, CAN_backplane_BUS_stats);	//Backplane BUS stats

}

/*
void smartkill_init(){
	pinMode(SA_EN, OUTPUT);
	pinMode(THRUST_EN, OUTPUT);
	DDRD = DDRD | B01000000; //set PD6 as output (TELE_EN)
	pinMode(POE_NAV, OUTPUT);
	pinMode(POE_SONAR, OUTPUT);
	pinMode(POE_ACOU, OUTPUT);
  
	//enable everything
	digitalWrite(SA_EN, HIGH);
	digitalWrite(THRUST_EN, HIGH);
	PORTD |= B01000000; //enable telemetry
	digitalWrite(POE_NAV, HIGH);
	digitalWrite(POE_SONAR, HIGH);
	digitalWrite(POE_ACOU, HIGH);
}

void checkSmartKill(){
	if((smartkill_buf & 0x01) == 1)	//bit 0 = thruster
		digitalWrite(THRUST_EN, LOW);	//kill thruster
	else
		digitalWrite(THRUST_EN, HIGH);	//enable thruster
	

	if(((smartkill_buf >> 1) & 0x01) == 1)	//bit 1 = SA
		digitalWrite(SA_EN, LOW);	//kill SA
	else
    digitalWrite(SA_EN, HIGH);  //enable SA
		
	if(((smartkill_buf >> 2) & 0x01) == 1)	//bit 2 = telemetery
		PORTD &= ~(B01000000);	//kill telemetery
	else
		PORTD |= B01000000;	//enable telemetery
	
}

*/
void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		if(CAN.getCanId() == CAN_backplane_kill){	//ID = 11
			//put into smartkill buffer
      Serial.println("RECEIVED!");
			smartkill_buf = buf[0];
		}else{
			//throw an error
		}

		CAN.clearMsg();
	}
}
