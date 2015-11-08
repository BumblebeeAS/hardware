#ifndef sensors_actuators_H_
#define sensors_actuators_H_
#include "Arduino.h"

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project bbauv_ardu_v1 here
int16_t readPressure();
void pressure_init();
void humid_init();
void readPressureFilter();
float readTempSensor(int8_t addr);
void readTemperature();
void readWater();
void topics_init();
void manipulators_init();
void leds_init();
#endif /* bbauv_ardu_v1_H_ */
