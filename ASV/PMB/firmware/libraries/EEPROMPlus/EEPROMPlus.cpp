#include "EEPROMPlus.h"

EEPROMPlus::EEPROMPlus(int i2c_address0, int i2c_address1){
  EEPROMAddrBuf = uint8_t(i2c_address0);
  EEPROMDataBuf = uint8_t(i2c_address1);
  addrPointer = 0;
  dataPointer = 0;
}

void EEPROMPlus::findStaringAddress(){
  bool found = false;
  //for addresses stored in the addres buffer as data
  uint8_t curr_addr, prev_addr = 0;
  //for the address pointer in the address buffer
  uint16_t temp_addr_pointer = 1;

  //start from the beginning
  Wire.beginTransmission(EEPROMAddrBuf);
  Wire.write((int)(0));   // MSB
  Wire.write((int)(0)); // LSB
  Wire.endTransmission();
  //initialise current address to the first address in the buffer 
  Wire.requestFrom(EEPROMAddrBuf, 2);
  if(Wire.available()) prev_addr = Wire.read();

  //go to the second address
  Wire.beginTransmission(EEPROMAddrBuf);
  Wire.write((int)(0));   // MSB
  Wire.write((int)(2)); // LSB
  Wire.endTransmission();
  while(!found){
    //32 is the maximum size of the i2c request buffer
    Wire.requestFrom(EEPROMAddrBuf, 32);
    while(Wire.available()){
      curr_addr = Wire.read();
      if(curr_addr != prev_addr + DATALEN){
        addrPointer = temp_addr_pointer;
        dataPointer = curr_addr;
        break;
      }
      prev_addr = curr_addr;
      temp_addr_pointer += ADDRLEN;
    } 
  }  
}

void EEPROMPlus::updateAddrBuff(){
  addrPointer += ADDRLEN;
  dataPointer += DATALEN;
  Wire.beginTransmission(EEPROMAddrBuf);
  Wire.write((int)(addrPointer >> 8));   // MSB
  Wire.write((int)(addrPointer & 0xFF)); // LSB
  Wire.endTransmission();
}

void EEPROMPlus::init(){
  Wire.begin();
  // findStaringAddress();
}

void EEPROMPlus::read(uint8_t* buf){
  uint8_t head = 0;
 
  Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(dataPointer >> 8));   // MSB
  Wire.write((int)(dataPointer & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(EEPROMDataBuf, DATALEN);
 
  while(Wire.available()){
    buf[head] = Wire.read();
    head++;
  } 
}


void EEPROMPlus::write(uint8_t* buf){
	Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(dataPointer >> 8));   // MSB
  Wire.write((int)(dataPointer & 0xFF)); // LSB
  Wire.write(buf, DATALEN);
  Wire.endTransmission();
  //update address pointer
  updateAddrBuff();

  delay(5);
}


uint8_t EEPROMPlus::dumbRead(uint16_t readAddr){
  uint8_t rdata = 0xFF;
 
  Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(readAddr >> 8));   // MSB
  Wire.write((int)(readAddr & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(EEPROMDataBuf,1);
 
  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}

void EEPROMPlus::dumbWrite(uint16_t writeAddr, uint8_t data){
  Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(writeAddr >> 8));   // MSB
  Wire.write((int)(writeAddr & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
 
  delay(5);
}

void EEPROMPlus::dumbMultiRead(uint16_t readAddr, uint8_t* buf){

  // Serial.println("dumbmultiread");
  uint8_t head = 0;
 
  Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(readAddr >> 8));   // MSB
  Wire.write((int)(readAddr & 0xFF)); // LSB
  Wire.endTransmission();
  // Serial.println("address sent");
  Wire.requestFrom(EEPROMDataBuf, DATALEN);
   // Serial.println("req sent");

  while(Wire.available()){
    // Serial.println("wire avail");

    buf[head] = Wire.read();
      // Serial.println(buf[head]);

    head++;
  }  
  // Serial.println("exit dumbmultiread");
}

void EEPROMPlus::dumbMultiWrite(uint16_t writeAddr, uint8_t* data){
  // Serial.println("dumbmultiwrite");
  Wire.beginTransmission(EEPROMDataBuf);
  Wire.write((int)(writeAddr >> 8));   // MSB
  Wire.write((int)(writeAddr & 0xFF)); // LSB
  Wire.write(data, DATALEN);
  Wire.endTransmission();
 
  delay(5);
}