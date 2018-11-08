//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        

// Written by Ng Ren Zhi

// Change Log for v1.0:
// - Initial commit

//###################################################
//###################################################

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define CAN_thruster	101
#define CAN_manual_thruster	102
#define CAN_control_link	103
#define CAN_heartbeat	104
#define CAN_soft_e_stop 105
#define CAN_e_stop		106
#define CAN_LARS	107
#define CAN_POPB_control	108
#define CAN_Shooter 109
#define CAN_LARS_stats 110
#define CAN_battery1_stats 111
#define CAN_battery2_stats 112
#define CAN_esc1_motor_stats 113
#define CAN_esc2_motor_stats 114
#define CAN_remote_kill_stats 115
#define CAN_INS_stats 116
#define CAN_GPS_stats 117
#define CAN_cpu_temp 118
#define CAN_POSB_stats 119
#define CAN_POPB_stats 120
#define CAN_POSB_BUS_stats 121
#define CAN_POKB_BUS_stats 122
#define CAN_POPB_BUS_stats 123
#define CAN_Tele_BUS_stats 124
#define CAN_LARS_BUS_stats 125
#define CAN_MANI_stats 126

//CAN Heartbeat
#define HEARTBEAT_POSB 1
#define HEARTBEAT_POPB 2
#define HEARTBEAT_POKB 3
#define HEARTBEAT_Tele 4
#define HEARTBEAT_LARS 5
#define HEARTBEAT_Cogswell 6
#define HEARTBEAT_OCS 7
#define HEARTBEAT_RC  8
//Bit position
#define HEARTBEAT_BATT1 0
#define HEARTBEAT_BATT2 1
#define HEARTBEAT_ESC1 2
#define HEARTBEAT_ESC2 3

#endif