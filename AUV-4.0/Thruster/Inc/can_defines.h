//###################################################
//###################################################
//
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
//
// Written by Chia Che
//
// Change log v1: initial commit
// Change log v1.1: add DNA stats, id = 10
//
//###################################################
//###################################################

#ifndef _DEFINE_H_
#define _DEFINE_H_

#define CAN_thruster_1 0
#define CAN_thruster_2 1
#define CAN_pressure 2
#define CAN_heartbeat 3
#define CAN_SONAR 4
#define CAN_PCB_Control 5
#define CAN_manipulator 6
#define CAN_ST_stats 7
#define CAN_PCB_stats 8
#define CAN_LED 9
#define CAN_DNA_Stats 10
#define CAN_PMB1_stats 11
#define CAN_PMB1_stats2 12
#define CAN_PMB2_stats 13
#define CAN_PMB2_stats2 14
#define CAN_CPU 15
#define CAN_MANI_BUS_stats 16
#define CAN_thruster_BUS_stats 17
#define CAN_ST_BUS_stats 18
#define CAN_PMB1_BUS_stats 19
#define CAN_PMB2_BUS_stats 20
#define CAN_SBC_BUS_stats 21
//===================================   can delete?
#define CAN_DVL_velocity 22
#define CAN_earth_odometry 23
#define CAN_relative_odometry 24
#define CAN_imu_1 25
#define CAN_imu_2 26

//CAN Heartbeat
#define HEARTBEAT_SBC 1
#define HEARTBEAT_SBC_CAN 2
#define HEARTBEAT_PCB 3
#define HEARTBEAT_TB 4
#define HEARTBEAT_ST 5
#define HEARTBEAT_MANI 6
#define HEARTBEAT_PMB1 7
#define HEARTBEAT_PMB2 8

#endif
