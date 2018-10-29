#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
}
// max 4095 == 5V
// 2252 == 2.749V
int DACinput = 2252;

// conversion is 
void loop() {
  byte addr = 0x4C;
  Wire.beginTransmission(addr);
  Wire.write(DACinput >> 8);     // top 4 bit of the 12bit voltage
  Wire.write(DACinput & 0xFF);    // bot 8 bit of the 12bit voltage
  Wire.endTransmission(true);
  
}
