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
//Firmware Version :             v1.1
//
// Written by Vanessa Cassandra
// Change log v1.1:
//  - Added OK indicator from heartbeat
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
Adafruit_RA8875 screen = Adafruit_RA8875(SCREEN_CS, SCREEN_RESET);	//screen
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
static uint8_t internalStats[11];
static uint16_t powerStats[7];
static uint8_t device;
static uint8_t dummy;
static uint8_t temp;
static double extPressure;
static uint32_t sa_timeout;
static uint32_t bp_timeout;
static uint32_t th_timeout;
static uint32_t pmb1_timeout;
static uint32_t pmb2_timeout;
static uint32_t heartbeat_timeout[9];
static uint32_t sbc_timeout;

static uint32_t loopTime;
static uint32_t logTime;
static uint32_t currentTime;
static uint32_t commandTime;
static uint32_t canStatsTime;
static uint32_t naTime;
static char presPrint[4];
static char dummyChar[9];
static String dummyStr;

double pressure;


int i = 0;
int cpu_core1;
int cpu_core2;

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
	// screen.writeReg(0x8E, 0x80 | 0x00);	//clear screen


	/*CHECK FOR CONNECTION*/
	if (millis() - sa_timeout > 5000){
    internalStats[INT_PRESS] = 255;
    internalStats[HUMIDITY] = 255;
    internalStats[SA_TEMP] = 255;
    sa_timeout = millis();
	}

  if(millis() - pmb1_timeout > 5000){
    internalStats[PMB1_PRESS] = 255;
    internalStats[PMB1_TEMP] = 255;
    powerStats[BATT1_CAPACITY] = 255;
    powerStats[BATT1_CURRENT] = 255;
    powerStats[BATT1_VOLTAGE] = 255;
    pmb1_timeout = millis();
  }

  if(millis() - pmb2_timeout > 5000){
    internalStats[PMB2_PRESS] = 255;
    internalStats[PMB2_TEMP] = 255;
    powerStats[BATT2_CAPACITY] = 255;
    powerStats[BATT2_CURRENT] = 255;
    powerStats[BATT2_VOLTAGE] = 255;
    pmb2_timeout = millis();
  }

  if(millis() - th_timeout > 5000){
    internalStats[TH_TEMP1] = 255;
    internalStats[TH_TEMP2] = 255;
    th_timeout = millis();
  }

  if(millis() - bp_timeout > 5000){
    internalStats[BP_TEMP] = 255;
    bp_timeout = millis();
  }

  if(millis() - sbc_timeout > 5000){
    internalStats[CPU_TEMP] = 255;
    sbc_timeout = millis();
  }


	if((millis() - loopTime) > 1000){
//	   	test_time = millis();
		screen_update();
//      Serial.println("TIME:");
//	   	Serial.println(millis() - test_time);
//		write_SD();
	    loopTime = millis();

	    /* CHECK FOR HEARTBEAT */
		for (int i = 1; i < 8; i++){
		if((millis() - heartbeat_timeout[i]) > 7000){
			screen_write_string(635, 175+35*i, "NO");
		}else
			screen_write_string(635, 175+35*i, "YES");
		}
	}

	checkCANmsg();

//	if((millis() - canStatsTime) > 1000){
//    test_time = millis();
//		//check CAN status and send back status
//		CAN_State_Buf[0]=CAN.checkError();
//		CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
//		CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
//		CAN.sendMsgBuf(CAN_telemetry_BUS_stats, 0, 3, CAN_State_Buf); //id, extended frame(1) or normal(0), no of bytes sent, data
//    Serial.print("CAN publish time:");
//    Serial.println(millis() - test_time);
//		canStatsTime = millis();
//	}
}

void write_SD(){
	// /**** WRITE INTO SD CARD ****/
	// myFile = SD.open("test.txt", FILE_WRITE);
	// myFile.write("testing 1, 2, 3.\n");
	
	// myFile.close();
}

void screen_init(){
	/* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
	if (!screen.begin(RA8875_800x480)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	screen.displayOn(true);
	screen.GPIOX(true);      // Enable screen - display enable tied to GPIOX
	screen.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	screen.PWM1out(255);
	screen.fillScreen(RA8875_GREEN);	//Fill screen with cyan color
  	delay(100);
	screen.fillScreen(RA8875_CYAN);
  	delay(100);
	screen.fillScreen(RA8875_BLUE);
  	delay(100);
	screen.fillScreen(RA8875_RED);
  	delay(100);
	screen.fillScreen(RA8875_RED);
	delay(100);
	screen.fillScreen(RA8875_MAGENTA);
	delay(100);
	screen.fillScreen(RA8875_YELLOW);
	delay(100);
	screen.fillScreen(RA8875_BLACK);

	/* Switch to text mode */  
	screen.textMode();

	/* Change the cursor location and color ... */  
	screen.textSetCursor(0, 12);
	screen.textEnlarge(4);
	screen.textTransparent(RA8875_YELLOW);

	/* ... and render some more text! */
	screen.textWrite("Bumblebee AUV");
}

void screen_prepare(){
	screen.fillScreen(RA8875_BLACK);
	screen.textEnlarge(1);
  	screen.textTransparent(RA8875_WHITE);
    screen.textSetCursor(0, 0);
    screen.textWrite("Ext press:");
    screen.textSetCursor(0, 40);
  	screen.textWrite("Int press:");
  	screen.textSetCursor(0, 80);
  	screen.textWrite("Humidity:");
    screen.textSetCursor(0, 120);
    screen.textWrite("PMB1 press:");
    screen.textSetCursor(0, 160);
    screen.textWrite("PMB2 press:");
    screen.textSetCursor(0, 200);
    screen.textWrite("PMB1 temp:");
    screen.textSetCursor(0, 240);
    screen.textWrite("PMB2 temp:");
    screen.textSetCursor(0, 280);
    screen.textWrite("Backplane temp:");
    screen.textSetCursor(0, 320);
    screen.textWrite("SA temp:");
    screen.textSetCursor(0, 360);
    screen.textWrite("Thruster temp1:");
    screen.textSetCursor(0, 400);
    screen.textWrite("Thruster temp2:");
    screen.textSetCursor(0, 440);
    screen.textWrite("CPU temp:");
    
    screen.textSetCursor(400, 0);
    screen.textWrite("Batt1 capacity:");
    screen.textSetCursor(400, 35);
    screen.textWrite("Batt2 capacity:");
    screen.textSetCursor(400, 70);
    screen.textWrite("Batt1 current:");
    screen.textSetCursor(400, 105);
    screen.textWrite("Batt2 current:");
    screen.textSetCursor(400, 140);
    screen.textWrite("Batt1 voltage:");
    screen.textSetCursor(400, 175);
    screen.textWrite("Batt2 voltage:");
    screen.textSetCursor(400, 210);
    screen.textWrite("SA OK:");
    screen.textSetCursor(400, 245);
    screen.textWrite("Thruster OK:");
    screen.textSetCursor(400, 280);
    screen.textWrite("Backplane OK:");
    screen.textSetCursor(400, 315);
    screen.textWrite("SBC-CAN OK:");
    screen.textSetCursor(400, 350);
    screen.textWrite("SBC OK:");
    screen.textSetCursor(400, 385);
    screen.textWrite("PMB1 OK:");
    screen.textSetCursor(400, 420);
    screen.textWrite("PMB2 OK:");
  	delay(10);
    
    screen.textTransparent(RA8875_YELLOW);
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
	screen_write(150, 440, internalStats[CPU_TEMP]);

	screen_write(645, 0, powerStats[BATT1_CAPACITY]);
	screen_write(645, 35, powerStats[BATT2_CAPACITY]);
	screen_write(635, 70, powerStats[BATT1_CURRENT]);
	screen_write(635, 105, powerStats[BATT2_CURRENT]);
	screen_write(635, 140, powerStats[BATT1_VOLTAGE]);
	screen_write(635, 175, powerStats[BATT2_VOLTAGE]);
}

void screen_write(uint32_t x, uint32_t y, uint32_t var){
	screen.fillRect(x, y, 130, 30, RA8875_BLACK);
  screen.textTransparent(RA8875_YELLOW);
	screen.textSetCursor(x, y);
  if (var == 255){
    screen_write_string(x, y, "N/A");
  }else{
    dummyStr = String(var);
    dummyStr.toCharArray(dummyChar, 9); //length = 9, arbitrary value
    screen.textWrite(dummyChar);
  }
}

void screen_write_string(uint32_t x, uint32_t y, const char* var){
  if(var == "YES"){
    screen.fillRect(x, y, 50, 30, RA8875_GREEN);
    screen.textTransparent(RA8875_BLACK);
  }
  else if(var == "NO"){
    screen.fillRect(x, y, 50, 30, RA8875_RED);
    screen.textTransparent(RA8875_WHITE);
  }
  else{
      screen.fillRect(x, y, 50, 30, RA8875_BLACK);
      screen.textTransparent(RA8875_YELLOW);
  }
	screen.textSetCursor(x, y);
	screen.textWrite(var);
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

//    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
//    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
//    
//    //register number, extension, 
//	CAN.init_Filt(0, 0, CAN_backplane_stats);
//	CAN.init_Filt(1, 0, CAN_SA_stats);
//	CAN.init_Filt(2, 0, CAN_thruster_stats);
//	CAN.init_Filt(3, 0, CAN_heartbeat);
//	CAN.init_Filt(4, 0, CAN_PMB1_stats3);
//	CAN.init_Filt(5, 0, CAN_PMB2_stats2);
//	CAN.init_Filt(6, 0, CAN_PMB2_stats3);
}

void checkCANmsg(){
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		switch(CAN.getCanId()){
		case CAN_pressure:
	        // pressure = buf[1]; //get first 8 bit
	        // pressure = pressure << 8;  //shift left
	        // pressure = pressure | buf[0];
	        Serial.print("Pressure: ");
	        // Case for Adafruit ADC raw current loop sensing
	        pressure = CAN.parseCANFrame(buf, 0, 2);
	        Serial.println(pressure);
	        pressure = 15*pressure/10684 - 7.498596031;
	        pressure*= 6895; //Convert to Pascals
	        extPressure = pressure + 99973.98; //plus ATM
	        break;

		case CAN_SA_stats:
			Serial.print("SA stats: ");
			sa_timeout = millis();
			internalStats[SA_TEMP] = CAN.parseCANFrame(buf, 0, 1);
			internalStats[HUMIDITY] = CAN.parseCANFrame(buf, 1, 1);
			internalStats[INT_PRESS] = CAN.parseCANFrame(buf, 2, 1);
	        Serial.println(internalStats[INT_PRESS]);
			break;

		case CAN_thruster_stats:
	        Serial.println("Thruster stats");
			internalStats[TH_TEMP1] = CAN.parseCANFrame(buf, 0, 1);
			internalStats[TH_TEMP2] = CAN.parseCANFrame(buf, 1, 1);
	        th_timeout = millis();
			break;

		case CAN_PMB1_stats2:
	        Serial.println("PMB1 stats 2");
			powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 4, 2);
			powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 6, 2);
	        pmb1_timeout = millis();
			break;

		case CAN_PMB1_stats3:
	        Serial.println("PMB1 stats 3");
			powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 2, 1);
			internalStats[PMB1_TEMP] = CAN.parseCANFrame(buf, 3, 1);
			internalStats[PMB1_PRESS] = CAN.parseCANFrame(buf, 4, 1);
			break;

		case CAN_PMB2_stats2:
	        Serial.println("PMB2 stats 2");
			powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 4, 2);
			powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 6, 2);
	        pmb2_timeout = millis();
			break;

		case CAN_PMB2_stats3:
	        Serial.println("PMB2 stats 3");
			powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 2, 1);
			internalStats[PMB2_TEMP] = CAN.parseCANFrame(buf, 3, 1);
			internalStats[PMB2_PRESS] = CAN.parseCANFrame(buf, 4, 1);
			break;
    
		case CAN_backplane_stats:
	        Serial.println("Backplane stats");
			internalStats[BP_TEMP] = CAN.parseCANFrame(buf, 0, 1);
	        bp_timeout = millis();
			break;

		case CAN_DVL_velocity:
	        Serial.println("DVL velocity");
			break;

		case CAN_earth_odometry:
	        Serial.println("Odometry");
			break;

		case CAN_relative_odometry:
	        Serial.println("Relative odometry");
			break;

		case CAN_imu_1:
	        Serial.println("IMU 1");
			break;
		
		case CAN_imu_2:
	        Serial.println("IMU 2");
			break;

		case CAN_heartbeat:
			device = CAN.parseCANFrame(buf, 0, 1);
			Serial.print("heartbeat: ");
			heartbeat_timeout[device] = millis();
			break;

    case CAN_CPU:
      cpu_core1 = CAN.parseCANFrame(buf, 1, 1);
      cpu_core2 = CAN.parseCANFrame(buf, 2, 1);
      if (cpu_core1 > cpu_core2)
        internalStats[CPU_TEMP] = cpu_core1;
       else
        internalStats[CPU_TEMP] = cpu_core2;
      Serial.print("CPU temp:");
      Serial.println(internalStats[CPU_TEMP]);
      sbc_timeout = millis();
      break;
      
		default:
			Serial.println("Others");
			break;
			}

		CAN.clearMsg();
	}
}
