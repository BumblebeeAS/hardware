#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  
}
// conversion is 
void loop() {
  byte addr = 0x4C;

  Wire.beginTransmission(addr);
  Wire.write(0xb);     // top 4 bit of the 12bit voltage
  Wire.write(0xce);    // bot 8 bit of the 12bit voltage
  Wire.endTransmission(true);
  
}
