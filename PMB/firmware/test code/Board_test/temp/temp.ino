#include <TempAD7414.h>
#include <Wire.h>

TempAD7414 temp_sens(96, 0x4e);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  temp_sens.initTempAD7414();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(temp_sens.getTemp());
  delay(1000);
}
