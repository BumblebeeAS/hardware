#include "Wire.h"
#include <HIH613x.h>

HIH613x HIH613x(0x27);

void setup(){

}

void loop(){

InternalPressure.updates();
delay(500);

}
