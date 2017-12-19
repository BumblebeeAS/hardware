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
//POSB for BBASV 2.0
//Firmware Version :             v1.0
//POSB Firmware for ASV 2.0
//		Telemetry LCD Display
//		Frisky Receiver (CPPM & DAC)
//		XBee
//
// Written by Ren Zhi
// Change log v0.0:
//
//###################################################
//###################################################
//###################################################

#include "LCD_Driver.h"
//#include "Telemetry.h"
//#include "define.h"
#include "can_defines.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>  //screen

Adafruit_RA8875 screen = Adafruit_RA8875(SCREEN_CS, SCREEN_RESET);  //screen
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
 
  screen_prepare();
}

int32_t test_time = 0;
void loop(){
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
//      test_time = millis();
    screen_update();
//      Serial.println("TIME:");
//      Serial.println(millis() - test_time);
//    write_SD();
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

//  if((millis() - canStatsTime) > 1000){
//    test_time = millis();
//    //check CAN status and send back status
//    CAN_State_Buf[0]=CAN.checkError();
//    CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
//    CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
//    CAN.sendMsgBuf(CAN_telemetry_BUS_stats, 0, 3, CAN_State_Buf); //id, extended frame(1) or normal(0), no of bytes sent, data
//    Serial.print("CAN publish time:");
//    Serial.println(millis() - test_time);
//    canStatsTime = millis();
//  }
}

//========= LCD ==========

void screen_prepare(){
  screen.fillScreen(RA8875_BLACK);
  screen.textEnlarge(1);
    screen.textMode();
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


//========= CAN ==========

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

    default:
      Serial.println("Others");
      break;
      }

    CAN.clearMsg();
  }
}