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
// BBAUV 3.99 actuation board
// Firmware Version :  v1.1
//
// Written by Titus for AUV 3.99 torpedo & dropper & grabber
// Adapted from BBAUV 4.0 actuation 
//
// Change log v1.1: 
// Update code to work with 1/4 step microstepping 
// PCB has been hacked for DRV8825 to be in 1/4 microstepping mode
// 
//###################################################
//###################################################

  
#include "can_defines.h" 
#include "can_auv_define.h" 
#include "define.h"
#include "can.h"
#include <Arduino.h>
#include <Servo.h>

// declare fuctions
void CAN_init();
void publishCanHB();
boolean receiveCanMessage();
void manipulators_init();
void dropper();
void top_torpedo();
void bot_torpedo();
void retract_grabber();
void extend_grabber();
void manipulate();
void reset_manipulate();

// Initialise servos for dropper and torpedo
Servo servo_dropper;
Servo servo_torpedo;

MCP_CAN CAN(8); //Set Chip Select to pin 8

uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

uint8_t maniControl = 0x00;  // Bit 0 to 7: RETXTXDX

// Initialise timeoout 
uint32_t dropperTimer = 0;
uint32_t torpedoTopTimer = 0;
uint32_t torpedoBotTimer = 0;
uint32_t serialStatusTimer = 0;
uint32_t heartbeatTimer = 0;

int fired_dropper = 0;
int fired_top = 0;
int fired_bot = 0;

int incomingByte = 0;   // for incoming serial data; test code

void setup() {
  Serial.begin(115200);
  Serial.println("Hi, I'm 3.99 Manipulator!");
  CAN_init(); //initialise CAN
  CANSetMask(); // CAN mask
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

  if ((millis() - serialStatusTimer) > SERIAL_STATUS_INTERVAL) {
    Serial.print("Manipulator Status: ");
    for (int i = 7; i >= 0; i--)
    {
      bool b = bitRead(maniControl, i);
      Serial.print(b);
    }
    Serial.println();
    serialStatusTimer = millis();
  }

  //Test code using Serial 
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.parseInt();

    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte);

    if (incomingByte == 1) //if 1 is typed in terminal, do top torpedo
    {
      maniControl |= FIRE_TOP_TORPEDO;
    }
    else if (incomingByte == 2) //if 2 is typed in terminal, do bot torpedo
    {
      maniControl |= FIRE_BOT_TORPEDO;
    }
    else if (incomingByte == 3) //if 3 is typed in terminal, do dropper
    {
      maniControl |= FIRE_DROPPER;
    }
    else if (incomingByte == 4) //if 4 is typed in terminal, retract grabber
    {
      maniControl |= RELEASE_GRABBER;
    }
    else if (incomingByte == 5) //if 5 is typed in terminal, extend grabber
    {
      maniControl |= ACTIVATE_GRABBER;
    }
    manipulate();
    reset_manipulate(); 
  }
}

void manipulators_init()
{
  // Initialize Manipulators
  servo_dropper.attach(DROP);
  servo_torpedo.attach(TORP);
  servo_dropper.write(125);
  servo_torpedo.write(95);
  // Declare stepper pins as output
  pinMode(STEP, OUTPUT);
  pinMode(DIR, OUTPUT);
}

void dropper()
{
  servo_dropper.write(65);
  fired_dropper = 1;
  dropperTimer = millis();
}

void top_torpedo()
{
  servo_torpedo.write(130);
  fired_top = 1;
  torpedoTopTimer = millis();
}

void bot_torpedo()
{
  servo_torpedo.write(65);
  fired_bot = 1;
  torpedoBotTimer = millis();
}

void extend_grabber() 
{
  // Set the spinning direction clockwise:
  digitalWrite(DIR, HIGH);
  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < STEPSPERREVOLUTION; i++) {
    // These four lines result in 1 step:
    digitalWrite(STEP, HIGH);
    delayMicroseconds(STEPPER_DELAY);
    digitalWrite(STEP, LOW);
    delayMicroseconds(STEPPER_DELAY);
  }
}

void retract_grabber()
{
  // Set the spinning direction counterclockwise:
  digitalWrite(DIR, LOW);
  // Spin the stepper motor 1 revolution quickly:
  for (int i = 0; i < STEPSPERREVOLUTION; i++) {
    // These four lines result in 1 step:
    digitalWrite(STEP, HIGH);
    delayMicroseconds(STEPPER_DELAY);
    digitalWrite(STEP, LOW);
    delayMicroseconds(STEPPER_DELAY);
  }
}

void manipulate() 
{
  if (maniControl == FIRE_TOP_TORPEDO) {
    top_torpedo();
    Serial.println("Fired top torpedo");
  }

  if (maniControl == FIRE_BOT_TORPEDO) {
    bot_torpedo();
    Serial.println("Fired bot torpedo");
  }

  if (maniControl == FIRE_DROPPER) {
    dropper();
    Serial.println("Fired dropper");
  }

  if (maniControl == RELEASE_GRABBER) {
    retract_grabber();
    Serial.println("Release grabber");
  }

  if (maniControl == ACTIVATE_GRABBER) {
    extend_grabber();
    Serial.println("Activated grabber");
  }
  maniControl = 0;
}

void reset_manipulate()
{
  if (fired_dropper && (millis() - dropperTimer) > DROPPER_INTERVAL)
  {
    servo_dropper.write(125);
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
  if (CAN_OK == CAN.begin(CAN_1000KBPS)) // init can bus : baudrate = 1000Kbps
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
  uint8_t HB[1] = { 6 }; //HEARTBEAT_MANI
  CAN.sendMsgBuf(CAN_heartbeat, 0, 1, HB);
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
  CAN.init_Mask(0, 0, 0xFF); //  check all bit 
  CAN.init_Filt(0, 0, 0x06); // let 00000110 pass (6) // Actuation  
}

boolean receiveCanMessage() { 
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBufID(&id, &len, buf);
    boolean messageForMani = false;
    switch (id) {
    case CAN_manipulator:
      maniControl = CAN.parseCANFrame(buf, 0, 1);
      messageForMani = true;
      if (DEBUG_MODE) {
        Serial.print("Mani Control Received: ");
        Serial.println(maniControl, HEX);
      }
      break;
    default:
      break;
    }
    CAN.clearMsg();
    return messageForMani;
  }

  else {
    return false;
  }
}
