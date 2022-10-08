//----------------------------------
// Testing code for controlling multiple tmc2209
// writen by Isabella
// v4.0
// 14 Sep 2022
// Log:
// - open and close established 
// - not all stallguard will trigger, might be checking at a too high frequency   
// - only need 2 stallguard: for pair 0&3 and pair 1&2
// - research on sfotware interupt for safe kill
// - implementing coolstep to save energy
// Stepper Position:
// Stepper 0 & 3 : long distance, moving out/forward when opening 
// Stepper 1 & 2 : short distance, moving in/backward when opening
//----------------------------------

//----defitions---------------------
#define SW_RX 6 //sw serial for uart line 1
#define SW_TX 7
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00
#define EN 5

#define microstep 16 
#define gSpeed 4000
// #define gTime 2000
#define STALL_VALUE 330

#define FORWARD 0
#define BACKWORD 1

#define OPENING 0
#define CLOSING 1

//-----------------------------------

#include <TMCStepper.h> // The library is edited

TMC2209Stepper stepper0 (SW_RX, SW_TX, R_SENSE, 0b00);
TMC2209Stepper stepper1 (SW_RX, SW_TX, R_SENSE, 0b01);
TMC2209Stepper stepper2 (SW_RX, SW_TX, R_SENSE, 0b10);
TMC2209Stepper stepper3 (SW_RX, SW_TX, R_SENSE, 0b11);

// Global Flags, 
int stepper_num = 4; // number of steppers 
int STALL_THRESHOLD[4] = {150, 220, 220, 150}; // higher = more sensitive

//TMC2209Stepper steppers[4] = {stepper0, stepper1, stepper2, stepper3};

void setup() {
  Serial.begin(250000);
  Serial.println( "Bienvenue. C'est DTLS.");

  //initialise dtls steppers
  dtls_init();
}

void loop() {

  while (Serial.available() > 0) {
    int8_t commandIndex = Serial.parseInt();

    switch (commandIndex)
    {
      case 0: // check UART status of all drivers 
        check_all_UART();
        break;
      case 1:
        parse_steppers_action(CLOSING);
        Serial.println("steppers closed.");
        break;
      case 2:
        parse_steppers_action(OPENING);
        Serial.println("steppers opened.");
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

  // move steppers to the open positions
  move_one_stepper(stepper0, FORWARD, STALL_THRESHOLD[0], gSpeed, 100000);
  move_one_stepper(stepper3, FORWARD, STALL_THRESHOLD[3], gSpeed, 100000);
  move_one_stepper(stepper1, BACKWORD, STALL_THRESHOLD[1], gSpeed, 100000);
  move_one_stepper(stepper2, BACKWORD, STALL_THRESHOLD[2], gSpeed, 100000);
}

void steppers_init(TMC2209Stepper s) {
  s.begin();
  s.blank_time(16);
  s.microsteps(microstep);
  s.VACTUAL(0);
}

void parse_steppers_action(int dir) {
  bool STALLED[2] = {false,false}; 
//  uint32_t now = millis();
//  uint32_t start_time = millis();

  if (dir) { // closing
     all_stepper_move(-gSpeed,gSpeed,gSpeed,-gSpeed);
  } else {  // opening
    all_stepper_move(gSpeed,-gSpeed,-gSpeed,gSpeed);
  }
  
  while (!STALLED[0] || !STALLED[1]) { 
        if (STALLED[0]) {
          // Serial.println("Stall detected for stepper 0 and 3");
          stepper0.VACTUAL(0);
          stepper3.VACTUAL(0);
        } else {
          STALLED[0] = stall_guard(stepper0, STALL_THRESHOLD[0]) || stall_guard(stepper3, STALL_THRESHOLD[3]);
        }
        
        if (STALLED[1]) {
          // Serial.println("Stall detected for stepper 1 and 2");
          stepper1.VACTUAL(0);
          stepper2.VACTUAL(0);
        } else {
          STALLED[1] = stall_guard(stepper1, STALL_THRESHOLD[1]) || stall_guard(stepper2, STALL_THRESHOLD[2]);
        }
        //now = millis();
  } // loop only exit when all motors stalled

  // a safety function that does nothing fundamentally
  all_stepper_stop();
}

void all_stepper_move(int speed0, int speed1, int speed2, int speed3) {
  stepper0.VACTUAL(speed0);
  stepper1.VACTUAL(speed1);
  stepper2.VACTUAL(speed2);
  stepper3.VACTUAL(speed3);
}

void all_stepper_stop() {
  stepper0.VACTUAL(0);
  stepper1.VACTUAL(0);
  stepper2.VACTUAL(0);
  stepper3.VACTUAL(0);
}
