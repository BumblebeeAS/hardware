//===========================================
//
//    DEFINES FOR ASV 3 TElEMETRY
//
//===========================================

#ifndef _DEFINES_H
#define _DEFINES_H

// ------------------ CAN ----------------------
#define CS_CAN 8


// ---------------- servo 1 (latch release for firing)-----------
#define LATCH_PIN 3
#define LATCH_START_ANGLE 180
#define LATCH_ENGAGE_ANGLE 150
#define LATCH_RELEASE_DELAY 300

// ---------------- servo 2 (drum servo)----------------------
#define DRUM_PIN 5
#define DRUM_START_ANGLE 100
#define DRUM_ENGAGE_ANGLE 100
#define DRUM_RELEASE_DELAY 100


// ------------------- linear actuator ------------------------
#define DIR_PIN 9
#define PWM_PIN 10

// ------------- tmc 2209 ------------------------------
#define SW_RX 6
#define SW_TX 7
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00
#define EN 2

#define microstep 16


#endif // _DEFINES_H
