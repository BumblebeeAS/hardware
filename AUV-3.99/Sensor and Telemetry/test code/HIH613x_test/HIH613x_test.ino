#include "Wire.h"
#include <HIH613x.h>

HIH613x sensor(0x27);

void setup(){
Serial.begin(115200);
Wire.begin();
}

void loop(){

sensor.measurementRequest();
delay(100);
sensor.dataFetch();
uint8_t humid = sensor.getHumidity()+0.5;
uint8_t temp = sensor.getTemperature()+0.5;
Serial.print(humid);
Serial.print("  ");
Serial.println(temp);
delay(100);

}
