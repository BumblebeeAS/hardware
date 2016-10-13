/*
EEPROMPlus.h
Author: Akshat
Library to provide more efficient space management in EEPROMs
*/

#include <Wire.h>
#include <Arduino.h>

//ALL DATA STORED MUST FOLLOW THE LENGTH
#define DATALEN 2 
#define ADDRLEN 2

class EEPROMPlus {
private:
  uint8_t EEPROMAddrBuf, EEPROMDataBuf = 0;
  uint16_t addrPointer, dataPointer = 0;
  
  void findStaringAddress();
  void updateAddrBuff();
public:
  EEPROMPlus(int i2c_address0, int i2c_address1);
	void init();
  //to read from the even loaded address
  void read(uint8_t* buf);
	//to write to the even loaded address
  void write(uint8_t* buf);
	uint8_t dumbRead(uint16_t readAddr);
	void dumbWrite(uint16_t writeAddr, uint8_t data);
  void dumbMultiRead(uint16_t readAddr, uint8_t* data);
  void dumbMultiWrite(uint16_t writeAddr, uint8_t* data);
};