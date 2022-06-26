#ifndef TELEM_SCREEN
#define TELEM_SCREEN

//#define DEBUG
#include <Adafruit_RA8875.h>
#include <Adafruit_GFX.h>
#include "Frisky_CPPM_ASV3.h"
#include "LCD_Driver.h"   
#include "define.h"

extern LCD screen;
extern Frisky frsky;
extern uint32_t internalStats[];
extern uint32_t powerStats[];
extern uint32_t heartbeat_timeout[];

void screen_prepare();
void screen_update_stats();
void screen_update_heartbeat();
void screen_update_hb();
void screen_reset_stats();
void reset_posb_stats();
void reset_ocs_stats();
void reset_rc_stats();
void reset_sbc_stats();
void reset_batt1_stats();
void reset_batt2_stats();

#endif
