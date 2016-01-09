#include <EEPROMPlus.h>

EEPROMPlus memory(0x50, 0x51);

void setup(void)
{
  Serial.begin(9600);
  memory.init();  
 
  unsigned int address = 0;
 
  memory.dumbWrite(address, 123);
  Serial.print(memory.dumbRead(address));
}
 
void loop(){}
