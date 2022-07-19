#define PMB_NO 5

//CAN pins
#define CAN_CS_PIN 8

//loop intervals
#define CAN_HEARTBEAT_INTERVAL 1000 
#define CAN_STATUS_INTERVAL 50
#define OLED_INTERVAL 1000

//GPIO CONNECTIONS
#define PIN_VEH_OFF 7    //PIN_PMOS
#define PIN_RELAY   A0   //PIN_RELAY
#define PIN_REED_OFF 3   //PIN_OFF
#define PIN_OLED_RESET 38

#define PMB_HEARTBEAT_ID 0x07 + (1-PMB_NO%2)  // 0x07 if PMB_NO is odd, 0x08 if even

//for internal sensors
#define Vref 5    //MPXH Vdd is the Vref
#define ADS_DELAY 5
