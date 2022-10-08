//----------------------------------
// Testing code for controlling multiple tmc2209
// writen by Isabella
// v1.1
// 22 Aug 2022
// Log: 
// - nil
// - adapting to dtls...
//----------------------------------

//----defitions---------------------
#define SW_RX0 6 //sw serial for uart line 1
#define SW_TX0 7
//#define SW_RX1 6 //sw serial for uart line 2
//#define SW_TX1 7
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00
#define EN 5

#define microstep 8
#define gSpeed 3000
#define gTime 1000
#define STALL_VALUE 250

//-----------------------------------

#include <TMCStepper.h>

TMC2209Stepper stepper0 (SW_RX0, SW_TX0, R_SENSE, 0b00);
TMC2209Stepper stepper1 (SW_RX0, SW_TX0, R_SENSE, 0b01);
TMC2209Stepper stepper2 (SW_RX0, SW_TX0, R_SENSE, 0b10);
TMC2209Stepper stepper3 (SW_RX0, SW_TX0, R_SENSE, 0b11);

//TMC2209Stepper[4] steppers {stepper0, stepper1, stepper2, stepper3};

void setup() {
  Serial.begin(115200);
  Serial.println( "Bienvenue. C'est 2209 multi slaves tester.");
  pinMode(5, OUTPUT);
  digitalWrite(5,LOW);

  //initialise steppers
  steppers_init(stepper0);
  steppers_init(stepper1);
  steppers_init(stepper2);
  steppers_init(stepper3);
}

void loop() {

  while (Serial.available() > 0) {
    int8_t commandIndex = Serial.parseInt();

    switch (commandIndex)
    {
      case 0:
        Serial.print(" Testing UART of Stepper 0: ...  ");
        stepper_talk(stepper0); 
        Serial.print(" Testing UART of Stepper 1: ...  ");
        stepper_talk(stepper1);
        Serial.print(" Testing UART of Stepper 2: ...  ");
        stepper_talk(stepper2);
        Serial.print(" Testing UART of Stepper 3: ...  ");
        stepper_talk(stepper3);
        Serial.println("UART test complete.");    
        break;
      case 1:
        digitalWrite(5, LOW);
        Serial.println ("Motors enabled");
        break;
      case 2:
        Serial.println("Motors disabled");
        digitalWrite(5, HIGH);
        break;
      case 3: 
      // stepper will move one after another, starting from 0 index stepper
        stepper_forward(stepper0);
        Serial.println("Stepper 0 moved forward" );
        stepper_forward(stepper1);
        Serial.println("Stepper 1 moved forward" );
        stepper_forward(stepper2);
        Serial.println("Stepper 2 moved forward" );
        stepper_forward(stepper3);
        Serial.println("Stepper 3 moved forward" );
        break;
      case 4:
        stepper_backward(stepper0);
        Serial.println("Stepper 0 moved backward" );
        stepper_backward(stepper1);
        Serial.println("Stepper 1 moved backward" );
        stepper_backward(stepper2);
        Serial.println("Stepper 2 moved backward" );
        stepper_backward(stepper3);
        Serial.println("Stepper 3 moved backward" );
        break;
      default:
        Serial.println("wrong command.");
        break;
    }

  }

}

void steppers_init(TMC2209Stepper s) {
  s.begin();
  s.blank_time(16);
  s.microsteps(microstep);
  s.VACTUAL(0);
}

bool stepper_talk(TMC2209Stepper s) {
  auto versionS = s.version();
  if (versionS == 0x21) {
    Serial.println("Success.");
    return true; //stepper talking through UART
  }

  else {
    uint8_t result = s.test_connection();
    if (result) {
      Serial.println("Failed!");
      Serial.print("Likely cause: ");

      switch (result) {
        case 1: Serial.println("loose connection"); break;
        case 2: Serial.println("no power"); break;
      }
      return false;
    }
    else {
      Serial.println("unknown error.");
      return false;
    }
  }
}

void stepper_forward(TMC2209Stepper s) {
  //move
  s.VACTUAL(gSpeed);
  delay(gTime);

  //stop
  s.VACTUAL(0);
}

void stepper_backward(TMC2209Stepper s) {
  //move
  s.VACTUAL(-gSpeed);
  delay(gTime);

  //stop
  s.VACTUAL(0);
}
