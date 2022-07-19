//===========================================
//
//    FRSKY HEADER FOR ASV 3 TElEMETRY
//
//===========================================

#ifndef TELEM_FRSKY
#define TELEM_FRSKY

// FOR DEBUG
//#define FRSKYDEBUG

#include "Frisky_CPPM_ASV3.h"
#include "define.h"
#include <Wire.h>

extern Frisky frsky;
extern uint32_t internalStats[];
extern uint32_t powerStats[];
extern uint32_t heartbeat_timeout[];
extern int control_mode_frsky;
extern bool remotekill_frsky;

void frsky_get_rssi();
int calculate_rssi();
void frsky_get_controlmode();
void frsky_send_batt_capacity();
void frsky_get_hydrophone();
void frsky_get_kill();

#endif
