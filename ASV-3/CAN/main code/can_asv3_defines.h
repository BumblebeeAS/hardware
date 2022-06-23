//###################################################
//###################################################

//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        
//
// Refer to CAN messaging standards: 
// https://docs.google.com/spreadsheets/d/1orc3DUXFo8djK77xSj1-ofPTmk6KvrHwtnzaI6hUamg/edit#gid=219917511
// Change Log for v1.0:
// - Initial commit
//###################################################
//###################################################

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define CAN_THRUSTER 0
#define CAN_MANUAL_THRUSTER 1
#define CAN_CONTROL_LINK 2
#define CAN_HEARTBEAT 3
#define CAN_SOFT_E_STOP 4
#define CAN_E_STOP 5 

#define CAN_POPB_CONTROL 7
#define CAN_BALL_SHOOTER 8
#define CAN_BALL_SHOOTER_STATS 9
#define CAN_BATT1_STATS 10
#define CAN_BATT2_STATS 11
#define CAN_ESC1_MOTOR_STATS 12
#define CAN_ESC2_MOTOR_STATS 13
#define CAN_ACOUSTICS_ACTUATION 14 
#define CAN_ACOUSTICS_ACTUATION_STATS 15

#define CAN_SBC_TEMP 17
#define CAN_POSB_STATS 18
#define CAN_POPB_STATS 19
#define CAN_WIND_SENSOR_STATS 20

#endif

