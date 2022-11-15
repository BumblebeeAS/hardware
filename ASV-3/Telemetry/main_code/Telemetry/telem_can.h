//===========================================x
//
//    CAN HEADER FOR ASV 3 TElEMETRY
//
//===========================================

// FOR DEBUG
//#define CANDEBUG

#ifndef TELEM_CAN
#define TELEM_CAN

//#define CANDEBUG

#include <can_asv3_defines.h>
#include "define.h"
#include <SPI.h> 
#include <can.h>

extern MCP_CAN CAN;
extern uint32_t id;
extern uint8_t len;                     //length of CAN message, taken care by library
extern uint8_t buf[];                                 //Buffer for CAN message
extern uint32_t internalStats[INT_STAT_COUNT];        // Array for internal stats
extern uint32_t powerStats[POWER_STAT_COUNT];         // Array for power stats
extern uint32_t heartbeat_timeout[HB_COUNT];          // Array for heartbeat timeout 
extern int control_mode;
extern uint32_t posb_timeout;
extern uint32_t sbc_timeout;
extern uint32_t batt1_timeout;
extern uint32_t batt2_timeout;
extern bool frsky_kill;
extern bool OCS_kill;
extern bool SBC_kill;
extern bool hard_kill;
extern bool radio_kill;

void CAN_init();
void CAN_mask();
void CAN_read_msg();
void read_heartbeat();
void read_batt_stats(int batt_no);
void read_posb_stats();
void CAN_publish_hb(int hb);
void CAN_publish_controllink();
void CAN_publish_kill();
void CAN_publish_hydroact(int is_kill);



#endif
