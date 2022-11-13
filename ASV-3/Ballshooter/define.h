//===========================================
//
//    DEFINES FOR ASV 3 TElEMETRY
//
//===========================================

#ifndef _DEFINES_H
#define _DEFINES_H

#include <Servo.h>
#include "can_asv3_defines.h"
#include <SPI.h> 
#include <can.h>

// CAN 
#define CAN_Chip_Select 54
MCP_CAN CAN(CAN_Chip_Select);
uint32_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

// stepper 2 (cocking)
#define dir2cock 5
#define step2cock 4
#define MS12 25
#define MS22 26
#define stepsPerRevolution 200

// stepper 3 (cocking)
#define dir3cock 7
#define step3cock 6
#define MS13 28
#define MS23 29

int stepperdelay = 200;
#define stepsPerRevolution 200
#define microstep 16

// servo 3 (latch release for firing)
Servo servo_latch;
#define LATCH_PIN 44
#define START_ANGLE 180
#define ENGAGE_ANGLE 150
#define RELEASE_DELAY 300

#endif // _DEFINES_H
