//###################################################
//################################################### 
// VESC CAN Defines
// Written by :
// Reference  : https://vesc-project.com/sites/default/files/imce/u15301/VESC6_CAN_CommandsTelemetry.pdf
//
// VESC uses an extended CAN frame where bit 0-7 is reserved for the esc id and bits 8 - 15 is used for the commands
// hence to send the corect CAN ID, the CAN ID is (COMMAND<<8 | ESC_ID)
//###################################################
//###################################################

#ifndef _VESC_DEFINE_H_
#define _VESC_DEFINE_H_

#define CAN_PACKET_SET_DUTY 0
#define CAN_PACKET_SET_CURRENT 1
#define CAN_PACKET_SET_CURRENT_BRAKE 2
#define CAN_PACKET_SET_RPM 3
#define CAN_PACKET_SET_POS 4
#define CAN_PACKET_FILL_RX_BUFFER 5
#define CAN_PACKET_FILL_RX_BUFFER_LONG 6
#define CAN_PACKET_PROCESS_RX_BUFFER 7
#define CAN_PACKET_PROCESS_SHORT_BUFFER 8
#define CAN_PACKET_STATUS 9
#define CAN_PACKET_SET_CURRENT_REL 10
#define CAN_PACKET_SET_CURRENT_BRAKE_REL 11
#define CAN_PACKET_SET_CURRENT_HANDBRAKE 12
#define CAN_PACKET_SET_CURRENT_HANDBRAKE_REL 13
#define CAN_PACKET_STATUS_2 14
#define CAN_PACKET_STATUS_3 15
#define CAN_PACKET_STATUS_4 16
#define CAN_PACKET_PING 17
#define CAN_PACKET_PONG 18
#define CAN_PACKET_DETECT_APPLY_ALL_FOC 19
#define CAN_PACKET_DETECT_APPLY_ALL_FOC_RES 20
#define CAN_PACKET_CONF_CURRENT_LIMITS 21
#define CAN_PACKET_CONF_STORE_CURRENT_LIMITS 22
#define CAN_PACKET_CONF_CURRENT_LIMITS_IN 23
#define CAN_PACKET_CONF_STORE_CURRENT_LIMITS_IN 24
#define CAN_PACKET_CONF_FOC_ERPMS 25
#define CAN_PACKET_CONF_STORE_FOC_ERPMS 26
#define CAN_PACKET_STATUS_5 27

#endif // _DEFINES_H
