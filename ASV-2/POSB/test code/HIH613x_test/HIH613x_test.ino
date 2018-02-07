#include "Wire.h"
#include <HIH613x.h>

HIH613x sensor(0x27);

void setup(){
Serial.begin(9600);
Wire.begin();
}

void loop(){

sensor.measurementRequest();
delay(100);
sensor.dataFetch();
delay(100);
uint8_t humid = sensor.getHumidity()+0.5;
uint8_t temp = sensor.getTemperature()+0.5;
Serial.println(humid);
Serial.println(temp);

delay(100);

}
