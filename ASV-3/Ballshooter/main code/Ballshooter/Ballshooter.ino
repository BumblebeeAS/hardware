//###################################################
//
//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        
//
// Ballshooter for BBASV 3.0
// Code for ASV 3-2 Ballshooter Board: 
// https://365.altium.com/files/5ACE52EF-0A3D-466A-820D-0B0AD493DB12?openedFrom=files
//
// Board functions: 
// 1. Run firing and cocking mechanism (1 latch release servo, 1 linear actuator)
// 2. Run reloading mechanism (1 stepper, 1 servo) 
// 
// Todo: 
// Add UART stepper code for drum
// Add servo movement for drum
// Add CAN heartbeat & receive
//
// Written by Titus Ng 
// Change log v0.1: 
// Inital commit 
//###################################################

#include "define.h" 
#define DEBUG
#include <can.h>
#include <can_asv3_defines.h>
#include <Servo.h>
#include <TMCStepper.h>

TMC2209Stepper drum_stepper (SW_RX, SW_TX, R_SENSE, 0b00);

MCP_CAN CAN(CS_CAN);
Servo servo_latch;

void setup() {
  initialize_firing();       // linear actuator + latch servo
  initialize_stepper();      // tmc2209
  initialize_drum_servo();   // drum servo
  initialize_serial();     
          
  // CAN init 
  CAN_init();
  CAN_mask();
}

void initialize_serial(void) {
  Serial.begin(115200);
  Serial.println("Hi I'm ASV 3 Ballshooter!");
  Serial.println("Serial controls:");
  Serial.println("1: Cock linear actuator");
  Serial.println("2: Unload linear actuator");
  Serial.println("3: Fire (release latch)");
  Serial.println("4: Reload drum");
}
//===========================================
//
//    Firing & Cocking initialization
//
//===========================================
void initialize_firing(void) {
  // linear actuator
  pinMode(DIR_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(PWM_PIN, LOW);
  // latch servo
  servo_latch.attach(LATCH_PIN);
  servo_latch.write(LATCH_START_ANGLE);
}

void initialize_drum_servo(void) {
  servo_latch.attach(DRUM_PIN);
  servo_latch.write(DRUM_START_ANGLE);
}

void initialize_stepper(void) {
  
}
//===========================================
//
//    Firing & Cocking movement
//
//===========================================
void release_latch(void) {
    servo_latch.write(LATCH_ENGAGE_ANGLE);
    delay(LATCH_RELEASE_DELAY);
    servo_latch.write(LATCH_START_ANGLE);
    Serial.println("3: Released latch");
}

void cock_act(){
  
}

void release_act() {
  
}

void reload_drum() {
  
}


void loop() {
  CAN_send_heartbeat();
  CAN_parse_command();
  
  #ifdef DEBUG
  int val = 0;
  val = Serial.parseInt();
  if (val != 0) {
    Serial.print("I got: ");
    Serial.println(val, DEC);
    if (val == 1) { 
      cock_act();
    } else if (val == 2) {
      release_act();
    } else if (val == 3) {
      release_latch();
    } else if (val == 4) {
      reload_drum();
    }
  }
  delay(100);
  #endif
}
