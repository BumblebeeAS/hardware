#ifndef Backplane_H_
#define Backplane_H_
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

void smartkill_init();
void smartkill_enable();
void checkCANmsg();

#endif /* bbauv_ardu_v1_H_ */
