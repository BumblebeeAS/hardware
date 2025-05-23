#define PMB_NO 4

#define CAN_HEARTBEAT_INTERVAL 500
#define CAN_STATUS_INTERVAL 1000
#define PRINT_INTERVAL 1000
#define OLED_INTERVAL 500
#define CURR_INTERVAL 10
#define VOLT_INTERVAL 20
#define TEMP_INTERVAL 1000
#define PRESSURE_INTERVAL 1000

#if (PMB_NO % 2 == 1)
#define PMB_HEARTBEAT_ID HEARTBEAT_PMB1
#define PMB_STATS_ID CAN_PMB1_stats
#define PMB_STATS2_ID CAN_PMB1_stats2
#else
#define PMB_HEARTBEAT_ID HEARTBEAT_PMB2
#define PMB_STATS_ID CAN_PMB2_stats
#define PMB_STATS2_ID CAN_PMB2_stats2
#endif

#define CAN_CS_PIN 8
#define MAX_CURR 80
#define SHUNT_RES 1 //mOhm
#define ARR_SIZE 10

//GPIO CONNECTIONS
#define PIN_PMOS 68
#define PIN_OLED_RESET 38
#define PIN_OFF 4
#define PIN_RELAY 61
#define MAX_CAPACITY 4290000000 //3600000000; // 10000 * 60 * 60 * 100 (scaling fact) (max of 2^32

//PMB 2 - 2755080000
//PMB 3 - 2718360000

//EEPROM CONSTANTS  
#define ADDR_CAPACITY 0
#define ADDR_VOLTAGE 4
#define VOLTAGE_DIFF 20 //200mV difference

//BATT_STATES
#define BATT_DISCHARGING 0
#define BATT_CHARGING 1
#define BATT_FULL 2

//TEMP_SENSOR
#define ADDR_TEMP_SENS 0X4E
#define CONFIG_TEMP_SENS 96

//PRESSURE_SENSOR
#define INTPRES_REF	5
