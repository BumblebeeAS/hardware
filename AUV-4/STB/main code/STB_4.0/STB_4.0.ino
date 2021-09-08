//###################################################
//###################################################
//
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
//
//Sensor and Telemetry for BBAUV 4.0
//Firmware Version :             v1.0
//
// Written by Yihang
//
//###################################################
//###################################################

#include <Wire.h>
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ADS1015.h>
#include <HIH613x.h>
#include "LCD_Driver.h"
#include <can_defines.h>
#include "define.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
//#include "can_auv_define.h"
#include "auv_4.0_can_def.h"
#include "MS5837.h"

//I2C
#define ADS_ADDR 0X48
#define HUMIDITY_ADDR 0X27

//CAN
#define CAN_Chip_Select 8
#define CAN_INT 2

//SCREEN
#define SCREEN_INT 3
#define SCREEN_CS 57
#define SCREEN_RESET 66
#define OFFSET 0 // 40 for ASV2.0 screen

//Sensor
#define Vref 5    //MPXH Vdd is the Vref
#define LPF_CONSTANT 0.7
#define ADS_DELAY 5
#define LPF_LOOP 25

//Internal stats
#define INT_STAT_COUNT 9

#define EXT_PRESS 0
#define INT_PRESS 1
#define PMB1_PRESS 2
#define PMB2_PRESS 3 
#define PMB1_TEMP 4
#define PMB2_TEMP 5
#define CPU_TEMP 6
#define HUMIDITY 7
#define ST_TEMP 8

//Power stats
#define POWER_STAT_COUNT 6

#define BATT1_CAPACITY 0
#define BATT2_CAPACITY 1
#define BATT1_CURRENT 2
#define BATT2_CURRENT 3
#define BATT1_VOLTAGE 4
#define BATT2_VOLTAGE 5

//Heartbeat
#define HB_COUNT 9

//TIMEOUTS
#define SCREEN_LOOP 1000
#define HB_TIMEOUT 3000
#define HEARTBEAT_LOOP 500
#define STAT_TIMEOUT 2000 // changed from 2000


// CAN variable
MCP_CAN CAN(CAN_Chip_Select);
uint32_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

//Screen variables
LCD screen = LCD(SCREEN_CS, SCREEN_RESET);
static uint16_t internalStats[INT_STAT_COUNT] = { 0 };
static uint16_t powerStats[POWER_STAT_COUNT] = { 0 };
static uint32_t heartbeat_timeout[HB_COUNT] = { 0 };
static uint32_t loopTime = 0;

//Sensor variables
Adafruit_ADS1115 ads(ADS_ADDR);
HIH613x humid(HUMIDITY_ADDR);
uint8_t humidity = 0;
uint8_t IntPressure = 0;
uint8_t temperature = 0;
uint16_t ExtPressure = 0;
uint8_t InitialP = 0;
uint16_t rawExtPressure = 0;
static uint32_t humidloop = 0;
bool readHumid = false;
static uint32_t pressure_loop = 0;
static uint32_t filter_loop = 0;
MS5837 sensor;


//Others
uint8_t CPU_CoreTemp = 0;
bool sonar = false;
static uint32_t pmb1_timeout = 0;
static uint32_t pmb2_timeout = 0;
static uint32_t sbc_timeout = 0;
static uint32_t dna_timeout = 0;
static uint32_t heartbeat_loop = 0;
static uint32_t stats_loop = 0;

static uint32_t testing_time = 0;

void setup()
{
  pinMode(SCREEN_CS, OUTPUT);       //CS screen
  digitalWrite(SCREEN_CS, HIGH);
  pinMode(CAN_Chip_Select, OUTPUT);   //CS CAN
  digitalWrite(CAN_Chip_Select, HIGH);

  Serial.begin(115200);
  Serial.println("Hi, I'm STB!");

  //Screen init
  screen.screen_init();
  Serial.println("Screen Ok");
  screen_prepare();

  //CAN init
  CAN_init();
  Serial.println("CAN OK");
  CANSetMask();

  //Sensor init
  Wire.begin();
  InitialP = readInternalPressure();
  Serial.println("int Sensors OK");
  if(!sensor.init()){
    Serial.println("External pressure Sensor Init failed!");
    Serial.println("Are SDA/SCL conected correctly?");
  }
  sensor.setModel(MS5837::MS5837_30BA);  
  sensor.setFluidDensity(997);
  Serial.println("depth OK");
  

  for (int i = 0; i < HB_COUNT; i++) {
    heartbeat_timeout[i] = millis();
  }
}

void loop()
{
  reset_stats();
  update_ST_stats();

  if ((millis() - loopTime) > SCREEN_LOOP) {  //  1000ms
    screen_update();
    update_heartbeat();
    loopTime = millis();
  }


  checkCANmsg();

  publishCAN();
}

//===========================================
//
//        CAN FUNCTIONS
//
//===========================================

void CAN_init() {
START_INIT:
  if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 1000k
#if DEBUG_MODE == NORMAL
    Serial.println("CAN init ok!");
#endif
  }
  else {
#if DEBUG_MODE == NORMAL
    Serial.println("CAN init fail");
    Serial.println("Init CAN again");
    delay(1000);
#endif
    goto START_INIT;
  }
}

void CANSetMask() {
  /*
  Truth table
  mask  filter    id bit  reject
  0     X         X       no
  1     0         0       no
  1     0         1       yes
  1     1         0       yes
  1     1         1       no
  Mask 0 connects to filt 0,1
  Mask 1 connects to filt 2,3,4,5
  Mask decide which bit to check
  Filt decide which bit to accept
  */


  //mask register 0
  CAN.init_Mask(0, 0, 0xFE); //  check 1111111X
  CAN.init_Filt(0, 0, 0x04); // let 0000010X pass (4 and 5)     // Heartbeat and SBC temp
  
  //mask register 1
  CAN.init_Mask(1, 0, 0xFF); // check 11111111
  CAN.init_Filt(2, 0, 0x17); // let 00010111 pass (23)    // BATT 1 STAT 
  CAN.init_Filt(3, 0, 0x18); // let 00011000 pass (24)    // PMB1 STAT
  CAN.init_Filt(4, 0, 0x19); // let 00011001 pass (25)    // BATT 2 STAT 
  CAN.init_Filt(5, 0, 0x1A); // let 00011010 pass (26)    // PMB2 STAT
}
/* Receive these CAN ID
 *  4: Heartbeat
 *  5: CPU_TEMP
 *  23: BATT 1 STAT
 *  24: PMB1 STAT
 *  25: BATT 2 STAT
 *  26: PMB2 STAT
 */

void checkCANmsg() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
    switch (CAN.getCanId()) {
    case CAN_HEARTBEAT:
    {
      uint32_t device = CAN.parseCANFrame(buf, 0, 1);
      heartbeat_timeout[device] = millis();
      break;  
    }
//    case CAN_SONAR:
//    {
//      uint8_t temp = CAN.parseCANFrame(buf, 0, 1);
//      (temp == 1) ? sonar = true : sonar = false;
//      break;
//    }
//    case CAN_LED:
//    {
//      selfSetColour[0] = CAN.parseCANFrame(buf, 0, 1);
//      selfSetColour[1] = CAN.parseCANFrame(buf, 1, 1);
//      selfSetColour[2] = CAN.parseCANFrame(buf, 2, 1);
//#ifdef SOFTPWM
//      setcolour(selfSetColour[0], selfSetColour[1], selfSetColour[2]);
//#else
//      //led.colour(lightColour);
//#endif
//      break;
//    }
//    case CAN_DNA_Stats:
//      internalStats[DNA_PRESS] = CAN.parseCANFrame(buf, 0, 1);
//      sbc_timeout = millis();
//      break;
    case CAN_BATT1_STAT:
      powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 0, 2);
      powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 2, 2);
      powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 6, 2);
      pmb1_timeout = millis();
      break;
    case CAN_PMB1_STAT:
      internalStats[PMB1_PRESS] = CAN.parseCANFrame(buf, 2, 2);
      internalStats[PMB1_TEMP] = CAN.parseCANFrame(buf, 0, 2);
      pmb1_timeout = millis();
      break;
    case CAN_BATT2_STAT:
      powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 0, 2);
      powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 2, 2);
      powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 6, 2);
      pmb2_timeout = millis();
      break;
    case CAN_PMB2_STAT:
      internalStats[PMB2_PRESS] = CAN.parseCANFrame(buf, 2, 2);
      internalStats[PMB2_TEMP] = CAN.parseCANFrame(buf, 0, 2);
      pmb2_timeout = millis();
      break;
    case CAN_CPU_TEMP:
    {
      uint8_t temp[5] = { 0 };
      for (int i = 0; i < 5; i++) {
        temp[i] = CAN.parseCANFrame(buf, i, 1);
      }
      uint8_t max_temp = temp[0];
      for (int i = 1; i < 5; i++) {
        if (temp[i] >= max_temp) {
          max_temp = temp[i];
        }
      }
      CPU_CoreTemp = max_temp;
      internalStats[CPU_TEMP] = CPU_CoreTemp;
      sbc_timeout = millis();
      break;
    }
    default:
      break;
    }
    CAN.clearMsg();
  }
}

//publish raw pressure, heartbeat and stats to CAN bus
void publishCAN()
{
  //publish raw external pressure reading every 50ms
//  if (millis() - pressure_loop > 50) {
//    publishCAN_pressure();
//    pressure_loop = millis();
//  }

  //publish heartbeat every 500ms
  if (millis() - heartbeat_loop > 500) {
    publishCAN_heartbeat(5);
    heartbeat_loop = millis();
  }

  //publish ST stats every 50ms
  if (millis() - stats_loop > 50) {
    publishST_stats();
    stats_loop = millis();
  }
}

void publishCAN_heartbeat(int device_id)
{
  id = CAN_HEARTBEAT;
  len = 1;
  buf[0] = device_id;
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

//void publishCAN_pressure() {
//  CAN.setupCANFrame(buf, 0, 2, rawExtPressure);
//  CAN.sendMsgBuf(CAN_pressure, 0, 2, buf);
//}

void publishST_stats() {
  id = CAN_STB_SENS;
  len = 8;
  buf[6] = rawExtPressure;
  buf[7] = rawExtPressure>>8;
  buf[4] = temperature;
  buf[5] = temperature>>8;
  buf[2] = humidity;
  buf[3] = humidity>>8;
  buf[0] = IntPressure;
  buf[1] = IntPressure>>8;
  CAN.sendMsgBuf(CAN_STB_SENS ,0, 8, buf);
}


//==========================================
//
//        LCD FUNCTIONS
//
//==========================================

void screen_prepare() {
  screen.set_cursor(0 + OFFSET, 0);
  screen.write_string("Ext press:");
  screen.write_string("Int press:");
  screen.write_string("PMB1 press:");
  screen.write_string("PMB2 press:");
  screen.write_string("PMB1 temp:");
  screen.write_string("PMB2 temp:");
  screen.write_string("CPU temp:");
  screen.write_string("Humidity:");
  screen.write_string("ST temp:");
  screen.write_string("SBC OK:");
  screen.write_string("SBC-CAN OK:");

  screen.set_cursor(400 + OFFSET, 0);
  screen.write_string("Batt1 capacity:");
  screen.write_string("Batt2 capacity:");
  screen.write_string("Batt1 current:");
  screen.write_string("Batt2 current:");
  screen.write_string("Batt1 voltage:");  
  screen.write_string("Batt2 voltage:");
  screen.write_string("Thruster OK:");
  screen.write_string("Manipulator OK:");
  screen.write_string("PMB1 OK:");
  screen.write_string("PMB2 OK:");
}

void screen_update() {
  // row height 35,     increment_row()
  screen.set_cursor(200 + OFFSET, 0);
  for (int i = 0; i < INT_STAT_COUNT; i++)
  {
    screen.write_value_int(internalStats[i]);
  }

  screen.set_cursor(645 + OFFSET, 0);
  for (int i = 0; i < POWER_STAT_COUNT; i++)
  {
    if (i > BATT2_CAPACITY) {
      screen.write_value_with_dp(powerStats[i], 3);
    }
    else {
      screen.write_value_int(powerStats[i]);
    }
  }
}

void update_heartbeat()
{
  // row height 35,     increment_row()
  int i;
  screen.set_cursor(200 + OFFSET, 315);
  for (i = 1; i < 3; i++) {
    if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
      screen.write_value_string("NO");
//      Serial.print("timeout");
//      Serial.println(i);
    }
    else {
      screen.write_value_string("YES");
    }        
  }

  screen.set_cursor(645 + OFFSET, 210);
  for (i = 4; i < 9; i++) {
    if (i != 5) { // Skip ST HB
      if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
        screen.write_value_string("NO");
//        Serial.print("timeout");
//        Serial.println(i);
      }
      else
        screen.write_value_string("YES");
    }
  }
}

//reset pmb1 pmb2 and sbc stats
void reset_stats()
{
  reset_pmb1_stat();
  reset_pmb2_stat();
  reset_sbc_stat();
}

void reset_pmb1_stat() {
  if ((millis() - pmb1_timeout) > STAT_TIMEOUT) {
    internalStats[PMB1_PRESS] = 0xFFFF;
    internalStats[PMB1_TEMP] = 0xFFFF;
    powerStats[BATT1_CAPACITY] = 0xFFFF;
    powerStats[BATT1_CURRENT] = 0xFFFF;
    powerStats[BATT1_VOLTAGE] = 0xFFFF;
    pmb1_timeout = millis();
  }
}

void reset_pmb2_stat() {
  if ((millis() - pmb2_timeout) > STAT_TIMEOUT) {
    internalStats[PMB2_PRESS] = 0xFFFF;
    internalStats[PMB2_TEMP] = 0xFFFF;
    powerStats[BATT2_CAPACITY] = 0xFFFF;
    powerStats[BATT2_CURRENT] = 0xFFFF;
    powerStats[BATT2_VOLTAGE] = 0xFFFF;
    pmb2_timeout = millis();
  }
}

void reset_sbc_stat() {
  if ((millis() - sbc_timeout) > STAT_TIMEOUT) {
    internalStats[CPU_TEMP] = 0xFFFF;
    sbc_timeout = millis();
  }
}

//read Temperature, Humidity, External and Internal pressure
// and assign them to array for update
void update_ST_stats() {
  readTempHumididty();
  rawExtPressure = readExternalPressure();
  internalStats[EXT_PRESS] = rawExtPressure;
  IntPressure = readInternalPressure();
  internalStats[INT_PRESS] = IntPressure;
  internalStats[HUMIDITY] = humidity;
  internalStats[ST_TEMP] = temperature;
}

//==========================================
//
//        Sensor Functions
//
//==========================================

//Return Internal Pressure
byte readInternalPressure() {
  /*
  VOUT = VS x (0.004 x P - 0.040)�� (Pressure Error x Temp Factor x 0.004 x VS)
  VS = 5.1 �� 0.36 Vdc
  */
  // internal   raw value 9690 = 1010mb = 101kPa
  ads.set_continuous_conv(1);
  delay(ADS_DELAY);
  uint16_t adc1 = ads.readADC_Continuous();

  return (((double)adc1*0.0001875) / (Vref*0.0040) + 10);
}

uint16_t readExternalPressure(){
  sensor.read();
  ExtPressure = sensor.pressure();
  return ExtPressure;
}

//Updates Temperature and Humidity
void readTempHumididty() {
  // reading temp or humid takes 36.65ms, 2 takes 74ms
  if (millis() - humidloop > 100) {
    if (!readHumid) {
      humid.measurementRequest();
      readHumid = true;
    }
    else {
      humid.dataFetch();
      humidity = humid.getHumidity() + 0.5;
      temperature = humid.getTemperature() + 0.5;
      readHumid = false;
    }
    humidloop = millis();
  }
}

//Return bool to indicate whether is it leaking
//Blinks led if it is leaking
bool leak() {
  bool leaking = false;
  if ((InitialP - IntPressure > 10) || humidity > 85) {
    leaking = true;
  }
  return leaking;
}
