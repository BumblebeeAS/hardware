/* Load Switch Pin Mappings */
#define DEBUG_MODE true

#define LS_NAVTIC 16
#define LS_VELODYNE 17
#define LS_OUSTER 18

/* Load Switch Bit Comparison */
#define ENABLE_OUSTER 0x01
#define ENABLE_VELODYNE 0x02
#define ENABLE_NAVTIC 0x04

#define STAGGER_TIME 250
#define AUTORESET_TIME 5000
#define CAN_HEARTBEAT_INTERVAL 500
#define CAN_STATUS_INTERVAL 1000
#define SERIAL_INTERVAL 2000
#define HEARTBEAT_POPB 0x02
