//===========================================
//
//    DEFINES FOR ASV 3 TElEMETRY
//
//===========================================

#ifndef _DEFINES_H
#define _DEFINES_H

// ------------------ CAN ----------------------
#define CS_CAN 8
#define COCK 1
#define LOAD 2
#define FIRE 3
#define ACK_COCK 4
#define ACK_LOAD 5
#define ACK_FIRE 6
#define BALL_HEARTBEAT 10
#define HB_TIMEOUT 500
// ---------------- servo 1 (latch release for firing)-----------
#define LATCH_PIN 3
#define LATCH_START_ANGLE 0
#define LATCH_ENGAGE_ANGLE 80
#define LATCH_RELEASE_DELAY 2000

// ---------------- servo 2 (drum servo)----------------------
#define DRUM_PIN 5
#define DRUM_START_ANGLE 0
#define DRUM_ENGAGE_ANGLE 180
#define DRUM_RELEASE_DELAY 2000

// ------------------- linear actuator ------------------------
#define DIR_PIN 9
#define PWM_PIN 10
#define DELAY_SHOOTER_RETRACT 3550
#define DELAY_SHOOTER_EXTEND 4000

// ------------- tmc 2209 ------------------------------
#define SW_RX 6
#define SW_TX 7
#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00
#define EN_PIN 2

#define microstep 8

#define STEPPER_RUNSPEED 1024
#define STEPPER_RUNTIME 540


#endif // _DEFINES_H
