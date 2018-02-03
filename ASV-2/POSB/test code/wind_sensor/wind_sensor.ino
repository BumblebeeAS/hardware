#include "define.h"
#include "wind_sensor.h"

WindSensor windSensor = WindSensor();

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  windSensor.readValues();
  Serial.print("Direction: ");
  Serial.print(windSensor.getDirection());
  Serial.print(" ");
  Serial.print("Speed: ");
  Serial.println(windSensor.getWindSpeed());
}
