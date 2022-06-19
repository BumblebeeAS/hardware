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
// BBAUV 4.0 Actuation
// Firmware Version : v2.12
// 
// Written by Linxin
// Edited by Titus   
// Change log v2.12
// Update to Khangs ball dropper
// 
//###################################################
//###################################################

#define DEBUG
#include "can_defines.h"
#include "auv_4.0_can_def.h" //update to current version
#include "define.h"
#include <can.h>
#include <SPI.h>
#include <can_defines.h>
#include <Arduino.h>
#include <Servo.h>

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

MCP_CAN CAN(CAN_Chip_Select); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t maniControl = 0x00;  // Bit 0 to 7: RETXTXDX

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
    else if (incomingByte == 54) //if 6 is typed in terminal, extend gripper for bottle
    {
      maniControl |= BOTTLE_GRABBER;
      Serial.println("bottle!");
    }
    manipulate();
    reset_manipulate(); 
  }
  #endif

}

void manipulators_init()
{
  //Initialize Manipulators
  servo_dropper.attach(DROP);
  servo_torpedo.attach(TORP);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  servo_dropper.write(10);     
  servo_torpedo.write(95);
}

void dropper()
{
  servo_dropper.write(173);
  fired_dropper = 1;
  dropperTimer = millis();
}

void top_torpedo()
{
  servo_torpedo.write(70);
  fired_top = 1;
  torpedoTopTimer = millis();
}

void bot_torpedo()
{
  servo_torpedo.write(120);
  fired_bot = 1;
  torpedoBotTimer = millis();
}

void activate_grabber() {
  // Set the spinning direction clockwise:
  digitalWrite(dirPin, LOW);

  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < stepsPerRevolution * microstep * 2.8;i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperdelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperdelay);
  }
}

void bottle_grabber() {
  digitalWrite(dirPin, LOW);
  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < stepsPerRevolution * microstep * 2.4;i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepperdelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepperdelay);
  }
  last_bottle = true;
}

void release_grabber(){  
  // Set the spinning direction counterclockwise:
  digitalWrite(dirPin, HIGH);

  if (!last_bottle) {
  // Close the grabber full
    for (int i = 0; i < stepsPerRevolution * microstep * 2.8; i++) {
      // These four lines result in 1 step:
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepperdelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepperdelay);
    }
  }
  else {
      // Close the grabber (it is not fully closed) 
    for (int i = 0; i < stepsPerRevolution * microstep * 2.4; i++) {
      // These four lines result in 1 step:
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepperdelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepperdelay);
    }
  }
  last_bottle = false;
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

  if (maniControl & BOTTLE_GRABBER) {
    bottle_grabber();
    Serial.println("Bottle grabber");
  }
  
  maniControl = 0;
}

void reset_manipulate()
{
  if (fired_dropper && (millis() - dropperTimer) > DROPPER_INTERVAL)
  {
    servo_dropper.writeMicroseconds(850);     
    fired_dropper = 0;
    Serial.println("Closed dropper");
  }

  if (fired_top && (millis() - torpedoTopTimer) > TORPEDO_INTERVAL)
  {
    servo_torpedo.write(95);
    fired_top = 0;
    Serial.println("Closed top torpedo");
  }

  if (fired_bot && (millis() - torpedoBotTimer) > TORPEDO_INTERVAL)
  {
    servo_torpedo.write(95);
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
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&id, &len, buf);
    boolean messageForMani = false;
    switch (id) {
    case CAN_ACT_CONTROL:  //CAN_ACT_CONTROL
      maniControl = CAN.parseCANFrame(buf, 0, 1);
      messageForMani = true;
      #ifdef DEBUG
        Serial.print("Mani Control Received: ");
        Serial.println(maniControl, HEX);
      #endif
      break;
    default:
      #ifdef DEBUG
        //Serial.println(id);
      #endif
      break;
    }
    CAN.clearMsg();     
    return messageForMani;
  }

  else {
    return false;
  }
}

void CAN_set_mask(){
  // Need to initialize both masks for filtering to work. Mask 1 cannot be 00 if not nothing will be checked.
  CAN.init_Mask(0, 0, 0xFF); //  check all bit 
  CAN.init_Filt(0, 0, 0x02); // let 2 pass (actuation) 
  CAN.init_Mask(1, 0, 0xFF);

}
