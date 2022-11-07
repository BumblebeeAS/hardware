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
// Written by Titus Ng 
// Change log v1.0: 
// MVP ball shooter
// working and tested with software controls & logic backplane
// tested with reloader and latch system
// missing cock mechanism using linear acutator
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
Servo servo_drum;
bool retracted = false;    // IMPT! linear actuator must start at extended position

void setup() {
  initialize_serial();     
  initialize_firing();       // linear actuator + latch servo
  initialize_stepper();      // tmc2209
  initialize_drum_servo();   // drum servo
          
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
  servo_drum.attach(DRUM_PIN);
  servo_drum.write(DRUM_START_ANGLE);
}
void initialize_stepper(void) {
  // enable pin
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);
  drum_stepper.begin();
  drum_stepper.blank_time(16);
  drum_stepper.microsteps(microstep);
  drum_stepper.VACTUAL(0);
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

void retract_act(){      
  if (!retracted) {
    digitalWrite(DIR_PIN, LOW);
    digitalWrite(PWM_PIN, HIGH);
    delay(DELAY_SHOOTER_RETRACT);
    digitalWrite(PWM_PIN, LOW);
    Serial.println("1: Cocked shooter");
    retracted = true;
  } else {
    Serial.println("WHY U TRYNA BREAK BALLSHOOTER");
  }
}

void extend_act() {
  if (retracted) {
    digitalWrite(DIR_PIN, HIGH);
    digitalWrite(PWM_PIN, HIGH);
    delay(DELAY_SHOOTER_EXTEND);
    digitalWrite(PWM_PIN, LOW);
    Serial.println("2: Released shooter");
    retracted = false;
  } else {
    Serial.println("ACTL WON'T BREAK BALLSHOOTER BUT STILL ITS ALREADY EXTENDED");
  }
}

//===========================================
//
//    Reloading movement
//
//===========================================

void reload_drum() {
    servo_drum.write(DRUM_ENGAGE_ANGLE);
    delay(DRUM_RELEASE_DELAY);

    servo_drum.write(DRUM_START_ANGLE);
    delay(DRUM_RELEASE_DELAY);

    drum_stepper.VACTUAL(STEPPER_RUNSPEED);
    delay(STEPPER_RUNTIME); 
    drum_stepper.VACTUAL(0);

    Serial.println("4: Reload drum");
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
      retract_act();
    } else if (val == 2) {
      extend_act();
    } else if (val == 3) {
      release_latch();
    } else if (val == 4) {
      reload_drum();
    }
  }
  while (Serial.available()) Serial.read();
  #endif
}
