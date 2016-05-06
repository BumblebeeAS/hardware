#ifndef _SMC_DRIVER_V2_
#define _SMC_DRIVER_V2_

#include <Arduino.h>

class smcDriver{
private:
  unsigned char CRC7_POLY;
  unsigned char CRCTable[256];
  float _thrusterRatio[6];
  HardwareSerial * mySerial;
public:
  smcDriver(HardwareSerial *serial);
  void init();
  
  unsigned char getCRCForByte(unsigned char val);
  void buildCRCTable();
  unsigned char getCRC(unsigned char message[], unsigned char length);
  void sendCommand(unsigned char message[], unsigned char length);
  void exitSafeStart(uint8_t id);
  void setMotorSpeed(uint8_t id, int speed);
  void setThrusterRatio(float ratio[]);
};

#endif