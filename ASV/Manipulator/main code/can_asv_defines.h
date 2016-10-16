//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        

// Written by Goh Eng Wei

//Change Log for v1.2:
// - added in thruster power on/off

// Change Log for v1.1:
// - added in EB stats
// - added in Mani CAN stats
// - Updated numbers to include AUV

// Change Log for v1.0:
// - Initial commit

//###################################################
//###################################################

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define CAN_thruster 101
#define CAN_heartbeat 102
#define CAN_e_stop 103
#define CAN_thruster_power 104
#define CAN_manipulator 105
#define CAN_LED 106
#define CAN_EB_stats 107
#define CAN_thruster1_motor_stats 108
#define CAN_thruster1_battery_stats 109
#define CAN_thruster1_range_stats 110
#define CAN_thruster2_motor_stats 111
#define CAN_thruster2_battery_stats 112
#define CAN_thruster2_range_stats 113
#define CAN_PMB1_stats 114
#define CAN_PMB1_stats2 115
#define CAN_PMB1_stats3 116
#define CAN_PMB2_stats 117
#define CAN_PMB2_stats2 118
#define CAN_PMB2_stats3 119
#define CAN_EB_BUS_stats 120
#define CAN_PMB1_BUS_stats 121
#define CAN_PMB2_BUS_stats 122
#define CAN_Mani_BUS_stats 123

//CAN Heartbeat
#define HEARTBEAT_EB 1
#define HEARTBEAT_Manipulator 2
#define HEARTBEAT_PMB1 6
#define HEARTBEAT_PMB2 7

#endif