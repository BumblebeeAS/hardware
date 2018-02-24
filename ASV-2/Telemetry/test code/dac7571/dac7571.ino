#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Baudrate: 9600\twith \\r");
  Serial.println("Pls key in DAC value: (0 to 5V)");  
}
char buffer[10] = "";
int idx = 0;
int DACinput = 0;

// conversion is 
void loop() {
  byte addr = 0x4C;

  while(Serial.available())
  {
    byte input = Serial.read();
    buffer[idx] = input;
    if(input == '\r' || input == '\n')
    {
      buffer[idx] = '\0';
      DACinput = atof(buffer)*4096/5;
      Serial.println(DACinput);
      idx = 0;
    }
    else
    {
      idx++;
    }
  }
  Wire.beginTransmission(addr);
  Wire.write(DACinput >> 8);     // top 4 bit of the 12bit voltage
  Wire.write(DACinput & 0xFF);    // bot 8 bit of the 12bit voltage
  Wire.endTransmission(true);
  
}
