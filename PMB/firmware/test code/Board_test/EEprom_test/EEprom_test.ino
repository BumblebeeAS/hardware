#include <EEPROMPlus.h>

EEPROMPlus memory(0x50, 0x51);

void setup(void)
{
  Serial.begin(9600);
  memory.init();  
 
  uint16_t address = 5;
  Serial.print("Attempting single byte write of 123: ");
  memory.dumbWrite(address, 123);
  Serial.println(memory.dumbRead(address));

  uint8_t in_buf[2] = {102, 220};
  Serial.print("Attempting to write 2 bytes, 102, 220: ");
  memory.dumbMultiWrite(address, in_buf);
  uint8_t out_buf[2] = {0};
  memory.dumbMultiRead(address, out_buf);
  Serial.print(out_buf[0]);
  Serial.println(out_buf[1]);
}
 
void loop(){}
