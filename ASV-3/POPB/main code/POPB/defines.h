/* Load Switch Pin Mappings */
#define DEBUG_MODE false
//old
//#define LS_NAVTIC 16
//#define LS_VELODYNE 17
//#define LS_OUSTER 18

//new
#define LS_NAVTIC 5
#define LS_VELODYNE 6
#define LS_OUSTER 7

#define RL_SBC 17
#define RL_INVERTER 3

#define TEMP1 A0
#define TEMP2 A1

/* Load Switch Bit Comparison */
#define ENABLE_OUSTER 1
#define ENABLE_VELODYNE 2
#define ENABLE_NAVTIC 4
#define ENABLE_INVERTER 8

#define ENABLE_SBC 16

#define STAGGER_TIME 250
#define AUTORESET_TIME 5000
#define CAN_HEARTBEAT_INTERVAL 500
#define CAN_STATUS_INTERVAL 1000
#define RELAY_INTERVAL 1000
#define SERIAL_INTERVAL 2000
#define HEARTBEAT_POPB 0x02
#define HEARTBEAT_BATT1 0X0D
#define TEMP_INTERVAL 500
