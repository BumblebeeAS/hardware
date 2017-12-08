//###################################################
//###################################################
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
// Written by Goh Eng Wei

// Change Log for v1.3:
// - Updated with new Heartbeat types
// Change Log for v1.2:
// - Updated with CPU Temp
// - re-ordered priorities for CAN Messages

// Change Log for v1.1:
// - Updated with Heartbeat CAN Message
// - re-ordered priorities for CAN Messages
//###################################################
//###################################################
//###################################################
#ifndef _DEFINE_H_
#define _DEFINE_H_


#define CAN_thruster_1 0
#define CAN_thruster_2 1
#define CAN_pressure 2
#define CAN_heartbeat 3
#define CAN_backplane_kill 4
#define CAN_manipulator 5
#define CAN_SA_stats 6
#define CAN_LED 7
#define CAN_thruster_stats 8
#define CAN_telemetry_stats 9
#define CAN_PMB1_stats 10
#define CAN_PMB1_stats2 11
#define CAN_PMB1_stats3 12
#define CAN_PMB2_stats 13
#define CAN_PMB2_stats2 14
#define CAN_PMB2_stats3 15
#define CAN_CPU 16
#define CAN_backplane_stats 17
#define CAN_backplane_stats2 18
#define CAN_backplane_BUS_stats 19
#define CAN_SA_BUS_stats 20
#define CAN_thruster_BUS_stats 21
#define CAN_telemetry_BUS_stats 22
#define CAN_PMB1_BUS_stats 23
#define CAN_PMB2_BUS_stats 24
#define CAN_SBC_BUS_stats 25
#define CAN_DVL_velocity 26
#define CAN_earth_odometry 27
#define CAN_relative_odometry 28
#define CAN_imu_1 29
#define CAN_imu_2 30

//CAN Heartbeat
#define HEARTBEAT_SA 1
#define HEARTBEAT_THRUSTER 2
#define HEARTBEAT_BACKPLANE 3
#define HEARTBEAT_SBC_CAN 4
#define HEARTBEAT_SBC 5
#define HEARTBEAT_PMB1 6
#define HEARTBEAT_PMB2 7
#define HEARTBEAT_TELEMETRY 8

#endif