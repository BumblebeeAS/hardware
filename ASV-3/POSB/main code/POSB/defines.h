#ifndef _DEFINES_H_
#define _DEFINES_H_

// Sensor polling
#define Vref 5
#define THRUSTER_STAT_TIMEOUT 200
#define SENSOR_POLL_TIMEOUT 500
#define BATT_TIMEOUT 333
#define ADS_DELAY 5

// Publish CAN
#define POWER_TIMEOUT 250
#define HEARTBEAT_TIMEOUT 500
#define WINDSPEED_TIMEOUT 250
#define THRUSTER_TIMEOUT 100
#define SENSOR_TIMEOUT 1000
#define HEARTBEAT_POSB 1
#define HEARTBEAT_BATT1 13
#define HEARTBEAT_BATT2 14
#define HEARTBEAT_ESC1 15
#define HEARTBEAT_ESC2 16

// Reset
#define INACTIVITY_TIMEOUT 2000
#define CONTROLMODE_TIMEOUT 3000
#define OFF_TIMEOUT 3000

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
