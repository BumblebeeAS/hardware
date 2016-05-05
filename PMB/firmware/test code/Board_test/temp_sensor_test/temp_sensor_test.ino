#include <TempAD7414.h>
#include <Wire.h>

TempAD7414 temp_sens(96, 0x4e);
void setup() {
  Serial.begin(9600);  
  temp_sens.initTempAD7414();
}

void loop() {
  Serial.println(temp_sens.getTemp());
  delay(1000);
}
