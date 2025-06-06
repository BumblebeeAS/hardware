//###################################################
//
//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        
//
// Telemetry for BBASV 3.0
//    Goals:
//    Drive Telemetry LCD Display using data received over CAN                          Done 
//    Recieve Thruster, actuated hydrophone and ball shooter controls from Frsky        Done (left actuated hydrophone and ball shooter)
//    Receive OCS control information from Controllink via serial.                      Not done -> thruster values? 
//    Relay RSSI of OCS and Frsky over CAN.                                             Frsky done OCS no
//    Determine state of control and send control signal to POSB accordingly.           done
//
// Written by Titus Ng 
// Change log v1.17:
// Read from mast radio
// Display mast radio status on screen
// remove ocs hb from screen
// mostly fix posb hb flickering
//  
//###################################################

// FOR DEBUG
#define DEBUG
#define RADIODEBUG

#include <Wire.h>
#include <Arduino.h>
#include "define.h"
#include "telem_screen.h"       
#include "telem_can.h"
#include "telem_frsky.h"        
#include "n2420.h"
 

// Create objects 
LCD screen = LCD(SCREEN_CS, SCREEN_RESET); 
Frisky frsky = Frisky(RC_INT);
MCP_CAN CAN(CAN_Chip_Select);

// CAN 
uint32_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

// Stats
uint32_t internalStats[INT_STAT_COUNT];
uint32_t powerStats[POWER_STAT_COUNT];
uint32_t heartbeat_timeout[HB_COUNT];

// Timeout
uint32_t posb_timeout = millis();
uint32_t ocs_timeout = millis();
uint32_t frsky_timeout = millis();
uint32_t sbc_timeout = millis();
uint32_t batt1_timeout = millis();
uint32_t batt2_timeout = millis();

// loop times
static uint32_t screenloop;
static uint32_t currentTime;
static uint32_t hbloop;
static uint32_t controllinkloop;
static uint32_t thrusterloop;
static uint32_t killloop;
static uint32_t hydrophoneloop;

// control 
bool frsky_alive = false;
bool frsky_kill = true;      // kill = true
bool OCS_kill = false;         // default false for now
bool SBC_kill = true;
bool hard_kill = true;
bool radio_kill = true;
int control_mode_frsky = AUTONOMOUS;
int control_mode = AUTONOMOUS;
int control_mode_ocs = AUTONOMOUS;
int16_t dir_forward = 0;
int16_t dir_side = 0;
int16_t dir_yaw = 0;
int16_t speed1 = 0;
int16_t speed2 = 0;
int16_t speed3 = 0;
int16_t speed4 = 0;
int frsky_timeout_count = 0;

// N2420 setup
N2420 n2420 (ASV_EXTENSION);
uint8_t inByte = 0;
uint8_t* inBuf;
uint32_t receiveRadioTime = 0;

void setup() {
  pinMode(SCREEN_CS, OUTPUT);           //CS screen
  digitalWrite(SCREEN_CS, HIGH);
  pinMode(CAN_Chip_Select, OUTPUT);     //CS CAN
  digitalWrite(CAN_Chip_Select, HIGH);

  Serial.begin(115200);
  Serial.println("Hi, I'm ASV 3 Telemetry!");

//  Screen init
  screen.screen_init();
  Serial.println("Screen Ok");
  screen_prepare();

  // CAN init 
  CAN_init();
  CAN_mask();

//  // frksy
  frsky.init();
//
  // DAC init
  Wire.begin();

  // initialise heartbeat 
  currentTime = screenloop = hbloop = millis();
  for (int i = 0; i < HB_COUNT; i++) {
    heartbeat_timeout[i] = millis();
  }

  // initalize radio 
    Serial2.begin(N2420_BAUD_RATE);
  n2420.setSerial(&Serial2);
  receiveRadioTime = millis();

}

void loop() {
//#ifdef DEBUG 
//  Serial.print("loop: ");
//  Serial.println(millis());
//#endif

  screen_reset_stats();   // reset stats if past timeout
  CAN_read_msg();         // read incoming CAN messages

  if (millis() - hydrophoneloop > HYDROPHONE_LOOP) {
    CAN_publish_hydroact(frsky_get_hydrophone_act());     // send actuation signal
    hydrophoneloop = millis();

  }
  
  if ((millis() - screenloop) > SCREEN_LOOP) {      // update screen
    screen_update_stats();
    screen_update_hb();
    screenloop = millis();
  }

  frsky_get_controlmode();        // get control mode from frsky 
  frsky_get_rssi();           
  frsky_send_batt_capacity();
  frsky_get_kill();
  get_directions();
  publish_controlmode();        // identify control mode based on control architecture
  set_thruster_values();        // transmit thruster commands

  if ((millis() - hbloop) > HEARTBEAT_LOOP) {     
    CAN_publish_hb(TELEMETRY);                          // Send telemetry hb 
    if (internalStats[RSSI_FRSKY] > RSSI_THRESHOLD && internalStats[RSSI_FRSKY] != 0xFFFF) {
      CAN_publish_hb(FRSKY);                            // send frsky hb if rssi is above threshold
      frsky_alive = true;
    }
    else {
      frsky_alive = false;
    }
    hbloop = millis();
  }

  if ((millis() - controllinkloop) > CONTROLLINK_LOOP) {      
    CAN_publish_controllink();                          // Send control link message
    controllinkloop = millis();
  }

  if ((millis() - thrusterloop) > THRUSTER_LOOP) {
    CAN_publish_manualthruster();
    thrusterloop = millis();
  }

  if ((millis() - killloop) > KILL_LOOP) {
    CAN_publish_kill();
    killloop = millis();
  }

  // Receive N2420 radio msg from mast
  if ((millis() - receiveRadioTime) > RECEIVE_RADIO_TIMEOUT) {
    receiveRadio();
    receiveRadioTime = millis();
  }
}

uint32_t last_response = millis();
//==========================================
//          RADIO FUNCTIONS
//==========================================
void receiveRadio() {
  n2420.readPacket();
  if (n2420.isAvailable()) {
#ifdef RADIODEBUG
    Serial.println("Response available.");
    Serial.print("ID: ");
#endif 
    int receiveID = n2420.getReceivingAddress();
#ifdef RADIODEBUG
    Serial.println(receiveID);
#endif
    // get data
    inBuf = n2420.showReceived();
    inByte = *(inBuf+2);
#ifdef RADIODEBUG
    Serial.print("inByte: ");
    Serial.println(inByte, HEX);
    uint32_t timebtwresponse = millis() - last_response;
    Serial.println(timebtwresponse);
    last_response = millis();
#endif
    if (receiveID == REMOTE_KILL || receiveID == OCS_EXTENSION){    //REMOTE_KILL is defined as 4 from library
      if (inByte == 0x15) {
#ifdef RADIODEBUG
       Serial.println("unkill");
#endif
       heartbeat_timeout[RADIO] = millis();
      } 
      else if (inByte == 0x44) {
#ifdef RADIODEBUG
        Serial.println("kill");
#endif
        heartbeat_timeout[RADIO] = millis();
      } 
      else {
#ifdef RADIODEBUG
        Serial.println("no data");
#endif
      }
    }
  }
}

//==========================================
//          THRUSTER FUNCTIONS
//==========================================

// Map CPPM values to thruster values
// CPPM: 980 to 2020 (neutral: 1500) (with deadzone from -24 to 24)
// Thruster: -3200 to 3200 (neutral: 0)
int32_t map_cppm(uint32_t value) {
  value = remove_deadzone(value);
  if (value >= 1500) {
    value = map(value, 1524, 2020, 0, 3200);  // Map values
  }
  else {
    value = map(value, 980, 1476, -3200, 0);
  }
  return value;
}

// Remove deadzone around 1500 (from +24 to -24)
uint32_t remove_deadzone(uint32_t value) {
  if (value >= 1500) {
    return constrain(value, 1524, 2020);
  }
  else {
    return constrain(value, 980, 1476);
  }
}

void get_directions() {
  dir_forward = map_cppm(frsky.get_ch(FRISKY_FORWARD));
  dir_side = map_cppm(frsky.get_ch(FRISKY_SIDE));
  dir_yaw = map_cppm(frsky.get_ch(FRISKY_YAW));
  #ifdef DEBUG
//  Serial.print("for: ");
//  Serial.println(dir_forward);
//  Serial.print("side: ");
//  Serial.println(dir_side);
//  Serial.print("yaw: ");
//  Serial.println(dir_yaw);
  #endif
}

void set_thruster_values() {
  if ((millis() - heartbeat_timeout[POKB]) > FAILSAFE_TIMEOUT) {
    // If no POKB heartbeat, stop all thrusters
    reset_thruster_values();
  }
  else  {
    switch (control_mode) {
    case AUTONOMOUS:
    case STATION_KEEP:
      reset_thruster_values();            // If not in manual mode, reset all thrusters
      break;
    case MANUAL_FRSKY: {
      convert_thruster_values();
      break;
    }
    }
  }
}
// Resolve vector thrust & Cap thrust at -3200 and 3200
void convert_thruster_values() {
  speed1 = constrain(-dir_forward - dir_side - dir_yaw, -3200, 3200);
  speed2 = constrain(-dir_forward + dir_side + dir_yaw, -3200, 3200);
  speed3 = constrain(dir_forward - dir_side + dir_yaw, -3200, 3200);
  speed4 = constrain(dir_forward + dir_side - dir_yaw, -3200, 3200);
}
void reset_thruster_values() {
  speed1 = 0;
  speed2 = 0;
  speed3 = 0;
  speed4 = 0;
}

//==========================================
//          CONTROL FUNCTIONS
//==========================================
void publish_controlmode() {
  // For control architure, refer to ASV 3 architure 
  // If Frsky alive, listen to Frsky
  if (frsky_alive) {
    control_mode = control_mode_frsky;
  } else {
    control_mode = STATION_KEEP;
  }

}

void CAN_publish_manualthruster()
{
  CAN.setupCANFrame(buf, 0, 2, (uint32_t)(speed1 + 3200));
  CAN.setupCANFrame(buf, 2, 2, (uint32_t)(speed2 + 3200));
  CAN.setupCANFrame(buf, 4, 2, (uint32_t)(speed3 + 3200));
  CAN.setupCANFrame(buf, 6, 2, (uint32_t)(speed4 + 3200));
  #ifdef DEBUG
//    Serial.print("1: ");
//    Serial.println(speed1 + 3200);
//    Serial.print("2: ");
//    Serial.println(speed2 + 3200);
//    Serial.print("3: ");
//    Serial.println(speed3 + 3200);
//    Serial.print("4: ");
//    Serial.println(speed4 + 3200);
  #endif
  CAN.sendMsgBuf(CAN_MANUAL_THRUSTER, 0, 8, buf);
}
