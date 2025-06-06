#ifndef _DEFINES_H_
#define _DEFINES_H_

// Sensor polling
#define THRUSTER_STAT_TIMEOUT 200
#define HUMID_TIMEOUT 500
#define BATT_TIMEOUT 333

// Publish CAN
#define POWER_TIMEOUT 250
#define HEARTBEAT_TIMEOUT 500
#define WINDSPEED_TIMEOUT 100
#define THRUSTER_TIMEOUT 100

// Reset
#define INACTIVITY_TIMEOUT 2000
#define CONTROLMODE_TIMEOUT 3000

// Light Tower
#define LIGHTTOWER_NONE 0
#define LIGHTTOWER_RED	58
#define LIGHTTOWER_YELLOW 59
#define LIGHTTOWER_GREEN 60
#define BLINK_TIMEOUT 500

// Control modes
#define AUTONOMOUS 1
#define MANUAL_RC 2
#define MANUAL_OCS 3
#define STATION_KEEP 4

#define XBEE_BAUDRATE 115200

#endif