//----------------------------------
// Testing code for controlling multiple tmc2209
// writen by Isabella
// v5.1
// 16 Nov 2022
// Log:
// - the code will send back a stepper fault msg back to can if
//   dtls is not closed/opened within 20s of the command
//----------------------------------

#include <TMCStepper.h> // The library is edited
#include "dtls_define.h"
#include <can.h>
#include <can_asv3_defines.h>

TMC2209Stepper stepper0 (SW_RX, SW_TX, R_SENSE, 0b00);
TMC2209Stepper stepper1 (SW_RX, SW_TX, R_SENSE, 0b01);
TMC2209Stepper stepper2 (SW_RX, SW_TX, R_SENSE, 0b10);
TMC2209Stepper stepper3 (SW_RX, SW_TX, R_SENSE, 0b11);

MCP_CAN CAN(CAN_CS_CHIP);

// Global Flags,
int stepper_num = 2; // number of steppers
int STALL_THRESHOLD[2] = {200, 200}; // higher = more sensitive

TMC2209Stepper steppers[4] = {stepper0, stepper1, stepper2, stepper3};
bool STALLED; 

void setup() {
  Serial.begin(115200);
  Serial.println(F("Bonjour. C'est DTLS de Isabella."));

  CAN_init();
  CAN_mask(); // to stop printing debug msg, comment out debug mode in can_define.h
  dtls_init();
  acous_init();
  retract_acous(); 
#ifdef DEBUG
  Serial.println(F("DTLS Set Up Completed. Checking UART"));
  check_all_UART(steppers, 4);
#endif
}

void loop() {

  CAN_send_heartbeat();
  CAN_parse_command();

#ifdef DEBUG
  while (Serial.available() > 0) {
    int8_t commandIndex = Serial.parseInt();

    switch (commandIndex)
    {
      case 0: // check UART status of all drivers
        check_all_UART(steppers, 4);
        break;
      case 1:
        parse_steppers_action(FORWARD);
        Serial.println("steppers moved forward.");
        break;
      case 2:
        parse_steppers_action(BACKWARD);
        Serial.println("steppers moved backward.");
        break;
      case 3: // enable all motors
        digitalWrite(5, LOW);
        Serial.println ("Motors enabled");
        break;
      case 4: // disable all motors
        Serial.println("Motors disabled");
        digitalWrite(5, HIGH);
        break;
      default:
        Serial.println("wrong command.");
        break;
    }
  }
#else 
  return;
#endif
}

void acous_init() {
  pinMode(ACOUS_DIR_PIN, OUTPUT);
  pinMode(ACOUS_PWM_PIN, OUTPUT);
  digitalWrite(ACOUS_DIR_PIN, LOW);
  digitalWrite(ACOUS_PWM_PIN, LOW);
}

void dtls_init() {
  //enabling steppers
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  // initialising steppers
  steppers_init(stepper0);
  steppers_init(stepper1);
  steppers_init(stepper2);
  steppers_init(stepper3);
#ifdef DEBUG
  Serial.println(F("===============DEBUG MODE==============="));
  Serial.println(F("Servers: initialising DTLS to open position"));
  // move steppers to the open positions
  parse_steppers_action(BACKWARD);
  Serial.println(F("DTLS Init Done"));
  Serial.println(F("========================================"));
#else 
  Serial.println(F("===============NORMAL MODE==============="));
  Serial.println(F("Servers: initialising DTLS to open position"));
  // move steppers to the open positions
  parse_steppers_action(BACKWARD);
  Serial.println(F("DTLS Init Done"));
  Serial.println(F("========================================"));
#endif
}

void steppers_init(TMC2209Stepper s) {
  s.begin();
  s.blank_time(16);
  s.microsteps(microstep);
  s.VACTUAL(0);
}
