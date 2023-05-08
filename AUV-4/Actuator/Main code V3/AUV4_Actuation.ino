//###################################################
//
// ____________________    _____   ____ _______   ____
// \______   \______   \  /  _  \ |    |   \   \ /   /
//  |    |  _/|    |  _/ /  /_\  \|    |   /\   Y   / 
//  |    |   \|    |   \/    |    \    |  /  \     /  
//  |______  /|______  /\____|__  /______/    \___/   
//         \/        \/         \/                    
// 
// BBAUV 4.0 Actuation
// Firmware Version : v3.0
//
// Grabber: Stepper
// Torpedo: Servo 2
// Dropper: Servo 1
//
// Written by Titus, Isabella
// Change log v3.3
// modify define file to suit: 
//  add checks to prevent actuator activation due to extended frame message of same CAN id sent to esc 2 
//###################################################

#define DEBUG
#include "auv_4.0_can_def.h" //update to current version
#include "define.h"
#include <can.h>
#include <SPI.h>
#include <Arduino.h>
#include <Servo.h>
#include <TMCStepper.h>

void CAN_init();
void publishCanHB();
boolean receiveCanMessage();
void manipulators_init();
void dropper();
void top_torpedo();
void bot_torpedo();
void retract_gripper();
void extend_gripper();
void manipulate();
void reset_manipulate();

Servo servo_dropper;      // servo 1
Servo servo_torpedo;      // servo 2
TMC2209Stepper grabber(SW_RX, SW_TX, 0.11, DRIVER_ADDRESS);

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t maniControl = 0x00;  // Bit 0 to  7: RETXTXDX

unsigned long dropperTimer = 0;
unsigned long torpedoTopTimer = 0;
unsigned long torpedoBotTimer = 0;
unsigned long serialStatusTimer = 0;
unsigned long heartbeatTimer = 0;

int fired_dropper = 0;
int fired_top = 0;
int fired_bot = 0;
bool last_bottle = false;

int incomingByte = 0;   // for incoming serial data of test code

void setup() {

  pinMode(CAN_Chip_Select, OUTPUT);     //CS CAN
  digitalWrite(CAN_Chip_Select, HIGH);
  Serial.begin(115200);
  Serial.println("Hi, I'm 4.0 Manipulator!");
  CAN_init(); //initialise CAN
  CAN_set_mask();
  heartbeatTimer = serialStatusTimer = millis();
  manipulators_init();
}

int32_t grabberSpeed = 0;

void loop()
{
  // Send MANI Heartbeat via CAN
  if ((millis() - heartbeatTimer) > HEARTBEAT_TIMEOUT) {
    publishCanHB();
    heartbeatTimer = millis();
  }

  //If received msg from CAN
  if (receiveCanMessage()) {
    manipulate();
  }
  reset_manipulate();


#ifdef DEBUG
  //Test code using Serial
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    //    // say what you got:
    //    Serial.print("I received: ");
    //    Serial.println(incomingByte, DEC);

    if (incomingByte == 49) //if 1 is typed in terminal, do top torpedo
    {
      maniControl |= FIRE_TOP_TORPEDO;
    }
    else if (incomingByte == 50) //if 2 is typed in terminal, do bot torpedo
    {
      maniControl |= FIRE_BOT_TORPEDO;
    }
    else if (incomingByte == 51) //if 3 is typed in terminal, do dropper
    {
      maniControl |= FIRE_DROPPER;
    }
    else if (incomingByte == 52) //if 4 is typed in terminal, close gripper
    {
      maniControl |= ACTIVATE_GRABBER;
    }
    else if (incomingByte == 53) //if 5 is typed in terminal, open gripper
    {
      maniControl |= RELEASE_GRABBER;
    }

    //testing, uart communication
    else if (incomingByte == 54) //if 6 is typed in terminal, stop motor
    {
      grabber_talk();
      grabberSpeed = 0;
      grabber.VACTUAL(grabberSpeed);
    }
    
    //testing, to take out
    else if (incomingByte == 55) //if 7 is typed in terminal, run motor
    {
      grabberSpeed = 5000;
      grabber.VACTUAL(grabberSpeed);
      grabber.VACTUAL(grabberSpeed); //to prevent stallguard overresponding
    }
    manipulate();
    reset_manipulate();
  }
#endif

  //testing, to take out
//  stall_guard();
}

void stall_guard(int stall_value) {     // different stall value based on closing or opening
  static uint32_t last_time = 0;
  uint32_t ms = millis();


  if ((ms - last_time) > 100 && grabberSpeed != 0) { // run every 0.1s
    last_time = ms;

    int load = (int) grabber.SG_RESULT();
    Serial.print("Status: ");
    Serial.println(load);
    if (load && load < stall_value )
    {
      grabberSpeed = 0;
      grabber.VACTUAL(grabberSpeed);
      Serial.println("Stall detected");
    }
  }
}

//testing, to take out
void grabber_talk() {
  auto versionG = grabber.version();
  if (versionG == 0x21) {
    Serial.println("Grabber talking through UART.");
  }
  else {
    Serial.print("\nTesting connection...");
    uint8_t result = grabber.test_connection();
    if (result) {
      Serial.println("failed!");
      Serial.print("Likely cause: ");

      switch (result) {
        case 1: Serial.println("loose connection"); break;
        case 2: Serial.println("no power"); break;
      }
    }
    else {
      Serial.println("UART not working, unknown error.");
    }
  }
}

void manipulators_init()
{
  //Initialize Servos
  pinMode(DROP, OUTPUT);
  pinMode(TORP, OUTPUT);
  servo_dropper.attach(DROP);
  servo_torpedo.attach(TORP);
  servo_dropper.write(10);
  servo_torpedo.write(TORP_RESET);

  //Intialize Stepper Driver
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  grabber.begin();
  grabber.blank_time(16);
  grabber.microsteps(microstep);
  grabber.VACTUAL(grabberSpeed);
}

void dropper()
{
  //  servo_dropper.write(DROP_FIRE);
  int fire_pos = DROP_FIRE;
  int reset_pos = DROP_RESET;
  for (double i = 1; i <= 1000; i++) {
    servo_dropper.write(fire_pos * (i / 1000));
    delayMicroseconds(DROPPER_STEP);
  }
  fired_dropper = 1;
  dropperTimer = millis();
}

void top_torpedo()
{
  servo_torpedo.write(TOP_FIRE);
  fired_top = 1;
  torpedoTopTimer = millis();
}

void bot_torpedo()
{
  servo_torpedo.write(BOT_FIRE);
  fired_bot = 1;
  torpedoBotTimer = millis();
}

void activate_grabber() {
  uint32_t ms = millis();
  uint32_t last_time = millis();
  
  // Spinning direction clockwise
  grabberSpeed = gSpeed;
  grabber.VACTUAL(grabberSpeed);
  grabber.VACTUAL(grabberSpeed); // to prevent stallguard overresponding

  while ((ms - last_time) < gTime) { // stop the grabber after ard 1.5 seconds
    ms = millis();
    stall_guard(STALL_VALUE_CLOSE);
  }

  // stop grabber
  grabber.VACTUAL(0);
  grabberSpeed = 0;
}

void release_grabber() {
  uint32_t ms = millis();
  uint32_t last_time = millis();
  
  // Spinning direction clockwise
  // motor velocity in +-(2^23)-1 [μsteps / t]
  grabberSpeed = -gSpeed;
  grabber.VACTUAL(grabberSpeed);
  grabber.VACTUAL(grabberSpeed); // to prevent stallguard overresponding

  while ((ms - last_time) < gTime) { // stop the grabber after ard 1.5 seconds
    ms = millis();
    stall_guard(STALL_VALUE_OPEN);
  }

  // stop grabber
  grabber.VACTUAL(0);
  grabberSpeed = 0;
}

void manipulate()
{
  if (maniControl & FIRE_TOP_TORPEDO) {
    top_torpedo();
    Serial.println("Fired top torpedo");
  }

  if (maniControl & FIRE_BOT_TORPEDO) {
    bot_torpedo();
    Serial.println("Fired bot torpedo");
  }

  if (maniControl & FIRE_DROPPER) {
    dropper();
    Serial.println("Fired dropper");
  }

  if (maniControl & ACTIVATE_GRABBER) {
    activate_grabber();
    Serial.println("Activated grabber");
  }

  if (maniControl & RELEASE_GRABBER) {
    release_grabber();
    Serial.println("Release grabber");
  }
  maniControl = 0;
}

void reset_manipulate()
{
  if (fired_dropper)
  {
    delay(DROPPER_INTERVAL);
    servo_dropper.write(DROP_RESET);
    delay(1000);
    fired_dropper = 0;
    Serial.println("Closed dropper");
  }

  if (fired_top)
  {
    delay(TORPEDO_INTERVAL);
    servo_torpedo.write(TORP_RESET);
    delay(500);
    fired_top = 0;
    Serial.println("Closed top torpedo");
  }

  if (fired_bot)
  {
    delay(TORPEDO_INTERVAL);
    servo_torpedo.write(TORP_RESET);
    delay(500);
    fired_bot = 0;
    Serial.println("Closed bot torpedo");
  }
}

void CAN_init()
{
START_INIT:
  if (CAN_OK == CAN.begin(CAN_500KBPS)) // init can bus : baudrate = 2220Kbps
  {
    Serial.println("CAN BUS: OK");
  }
  else
  {
    Serial.println("CAN BUS: FAILED");
    Serial.println("CAN BUS: Reinitializing");
    delay(1000);
    goto START_INIT;
  }
  Serial.println("INITIATING TRANSMISSION...");
}

void publishCanHB() {
  uint8_t HB[1] = { CAN_ACT_HEARTBEAT };  //CAN_ACT_HEARTBEAT
  CAN.sendMsgBuf(CAN_ACT_HEARTBEAT, 0, 1, HB); //CAN_HEARTBEAT
}

boolean receiveCanMessage() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBufID(&id, &len, buf);
    boolean is_ActMsg = false;
    switch (id) {
      case CAN_ACT_CONTROL: {  // CAN msg received is for actuation
          if (len == 1)
          {
            maniControl = CAN.parseCANFrame(buf, 0, 1);
            is_ActMsg = true;
#ifdef DEBUG
            Serial.print("Mani Control Received: ");
            Serial.println(maniControl, HEX);
#endif            
          }
          else
          {
            Serial.println("Wrong Mani command size");
            Serial.print("bytes received:"); 
            Serial.println(len);         
            Serial.println("Expected size: 1"); 
          }
          break;
        }
      default: {
#ifdef DEBUG
          //Serial.print("not Actuation Control, CAN id is: ")
          //Serial.println(id);
#endif
          break;
        }
    }
    CAN.clearMsg();
    return is_ActMsg;
  }

  else {
    return false;
  }
}

void CAN_set_mask() {
  // Need to initialize both masks for filtering to work. Mask 1 cannot be 00 if not nothing will be checked.
  CAN.init_Mask(0, 0, 0xFF); //  check all bit
  CAN.init_Filt(0, 0, 0x02); // let 2 pass (actuation)
  CAN.init_Mask(1, 0, 0xFF);

}
