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
// Sensor and Telemetry for BBAUV 4.0
// Firmware Version :             v1.7
//
// Written by Titus
// Change log v1.7:
// Remove screen LED functions
// Add Software PWM for LED
//
//###################################################
//###################################################

// FOR DEBUG
//#define DEBUG
#ifdef DEBUG
int timeout_count = 0;
#endif

#include <Wire.h>
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ADS1015.h>
#include <HIH613x.h>
#include <PalatisSoftPWM.h>
#include "LCD_Driver.h"
#include <can_defines.h>
#include "define.h"
#include <Arduino.h>
#include <SPI.h> //for CAN controller
#include <can.h>
#include "auv_4.0_can_def.h"
#include "MS5837.h"

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

//led variables
SOFTPWM_DEFINE_PIN24_CHANNEL(0);  //Arduino pin 24 as channel 0
SOFTPWM_DEFINE_PIN23_CHANNEL(1);  //Arduino pin 23 as channel 1
SOFTPWM_DEFINE_PIN22_CHANNEL(2);  //Arduino pin 22 as channel 2 
SOFTPWM_DEFINE_OBJECT(3);         // Initialize 3 channels

uint8_t led_state = OFF;

void setup()
{
  pinMode(SCREEN_CS, OUTPUT);           //CS screen
  digitalWrite(SCREEN_CS, HIGH);
  pinMode(CAN_Chip_Select, OUTPUT);     //CS CAN
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

  //led_init
  led_init();

  //Sensor init
  Wire.begin();
  Wire.setWireTimeout(500, true);          // Set i2c timeout for 500ms 
  InitialP = readInternalPressure();
  Serial.println("int Sensors OK");
  if(!sensor.init()){
    Serial.println("External pressure Sensor Init failed!");
    Serial.println("Are SDA/SCL conected correctly?");
  }
  sensor.setModel(MS5837::MS5837_30BA);  
  sensor.setFluidDensity(997);
  Serial.println("depth OK");
  
  // setup initial heartbeat timeout
  for (int i = 0; i < HB_COUNT; i++) {
    heartbeat_timeout[i] = millis();
  }
}

void loop()
{
  reset_stats();
  update_ST_stats();

  if ((millis() - loopTime) > SCREEN_LOOP) { 
    screen_update();
    update_heartbeat();
    loopTime = millis();
  }
  publishCAN();

  checkCANmsg();
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

/* Receive these CAN ID
 *  4: Heartbeat
 *  5: CPU_TEMP 
 *  12: LED - 00001100
 *  23: BATT 1 STAT
 *  24: PMB1 STAT
 *  25: BATT 2 STAT
 *  26: PMB2 STAT
 */

  //mask register 0
  CAN.init_Mask(0, 0, 0xFE); //  check 1111111X
  CAN.init_Filt(0, 0, 0x04); // let 0000010X pass (4 and 5)         // Heartbeat and SBC temp
  CAN.init_Filt(1, 0, 0x0C); // let 0x00110x pass (12 and 13)      // LED (13 is unused)
  
  //mask register 1
  CAN.init_Mask(1, 0, 0xFF); // check 11111111
  CAN.init_Filt(2, 0, 0x17); // let 00010111 pass (23)    // BATT 1 STAT 
  CAN.init_Filt(3, 0, 0x18); // let 00011000 pass (24)    // PMB1 STAT
  CAN.init_Filt(4, 0, 0x19); // let 00011001 pass (25)    // BATT 2 STAT 
  CAN.init_Filt(5, 0, 0x1A); // let 00011010 pass (26)    // PMB2 STAT
}


void checkCANmsg() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
    #ifdef DEBUG 
      //Serial.println(CAN.getCanId());`
    #endif 
    switch (CAN.getCanId()) {

    case CAN_HEARTBEAT:
    {
      uint32_t device = CAN.parseCANFrame(buf, 0, 1);
      heartbeat_timeout[device] = millis();
      break;   
    }
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
    case CAN_STB_UP: {
      int red = CAN.parseCANFrame(buf, 0, 1);
      int green = CAN.parseCANFrame(buf, 1, 1);
      int blue = CAN.parseCANFrame(buf, 2, 1);
      Serial.println(red);
      Serial.println(green);
      Serial.println(blue);
      setcolour(red, green, blue);
      break;
    }
    default:
      #ifdef DEBUG
        Serial.println(id);
      #endif
      break;
    }
    CAN.clearMsg();
  }
}

// publish raw pressure, heartbeat and stats to CAN bus
void publishCAN()
{
  if (millis() - heartbeat_loop > HEARTBEAT_LOOP) {
    publishCAN_heartbeat(5);
    heartbeat_loop = millis();
  }

  if (millis() - stats_loop > STATS_LOOP) {
    publishST_stats();
    stats_loop = millis();
  }
}

// publish own heartbeat
void publishCAN_heartbeat(int device_id)
{
  id = CAN_HEARTBEAT;
  len = 1;
  buf[0] = device_id;
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

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
  // display from Ext press to ST temp
  screen.set_cursor(200 + OFFSET, 0);
  for (int i = 0; i < INT_STAT_COUNT; i++)
  {
    // Display internal pressure as kpa with 1 dp
    if (i == 0) {
      screen.write_value_with_dp(internalStats[i], 1);
    } else if (i == 1) {
      // Display internal pressure in kpa with 2 dp
      float intP = readInternalPressure() * 100;
      screen.write_value_with_dp(intP, 2);
    } else {
      screen.write_value_int(internalStats[i]);
    }
  }

  // display from Batt 1 capacity to Batt 2 voltage
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

// display heartbeat status
void update_heartbeat() {
  int i;
  screen.set_cursor(200 + OFFSET, 315);
  // display SBC & SBC-CAN
  for (i = 1; i < 3; i++) {
    if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
      screen.write_value_string("NO");
    }
    else {
      screen.write_value_string("YES");
    }        
  }
  // display THRUSTER to PMB2
  screen.set_cursor(645 + OFFSET, 210);
  for (i = 4; i < 9; i++) {
    if (i != 5) { // Skip ST HB
      if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
        screen.write_value_string("NO");
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
  IntPressure = (byte)readInternalPressure();
  internalStats[INT_PRESS] = IntPressure;
  internalStats[HUMIDITY] = humidity;
  internalStats[ST_TEMP] = temperature;
}

//==========================================
//
//        Sensor Functions
//
//==========================================

//Return Internal Pressure in kpa
double readInternalPressure() {
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
 
// Return External Pressure in mbar
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

//==========================================
//
//        External LED Functions
//
//==========================================


void setcolour(int red, int green, int blue) {
  int scale = 2.5;
  PalatisSoftPWM.set(RED, red);
  PalatisSoftPWM.set(GREEN, green/scale);
  PalatisSoftPWM.set(BLUE, blue/scale);
}

//Blinks through all colour
void led_init() {
  PalatisSoftPWM.begin(200); //200Hz
  setcolour(255,0,0);
  delay(200);
  setcolour(255,255,0);
  delay(200);
  setcolour(0,255,0);
  delay(200);
  setcolour(0,255,255);
  delay(200);
  setcolour(0,0,255);
  delay(200);
  setcolour(255,0,255);
  delay(200);
  setcolour(0,0,0);
}
