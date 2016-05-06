#include "smcDriver_v2.h"

smcDriver::smcDriver(HardwareSerial *serial) 
{
  mySerial = serial;
  CRC7_POLY = 0x91;
  _thrusterRatio[0]=1;
  _thrusterRatio[1]=1;
  _thrusterRatio[2]=1;
  _thrusterRatio[3]=1;
  _thrusterRatio[4]=1;
  _thrusterRatio[5]=1;
}

void smcDriver::init()
{
  smcDriver::buildCRCTable();
  delay(5);
  
  exitSafeStart(1);
  exitSafeStart(2);
  exitSafeStart(3);
  exitSafeStart(4);
  exitSafeStart(5);
  exitSafeStart(6); 
}
unsigned char smcDriver::getCRCForByte(unsigned char val)
{
  unsigned char j;
 
  for (j = 0; j < 8; j++)
  {
    if (val & 1)
      val ^= CRC7_POLY;
    val >>= 1;
  }
 
  return val;
}
 
void smcDriver::buildCRCTable()
{
  int i;
 
  // fill an array with CRC values of all 256 possible bytes
  for (i = 0; i < 256; i++)
  {
    CRCTable[i] = smcDriver::getCRCForByte(i);
  }
}
 
unsigned char smcDriver::getCRC(unsigned char message[], unsigned char length)
{
  unsigned char i, crc = 0;
 
  for (i = 0; i < length; i++)
    crc = smcDriver::CRCTable[crc ^ message[i]];
  return crc;
}

//motorcontroller handling code!
//----------------------------------------------
void smcDriver::sendCommand(unsigned char message[], unsigned char length)
{
  unsigned char i=0;
  
  for(i=0;i<length;i++)
  {
    mySerial->write(message[i]);
  }
}

// required to allow motors to move
// must be called when controller restarts and after any error
void smcDriver::exitSafeStart(uint8_t id)
{
  unsigned char message[4] = {0xAA,0x00,0x03,0x00};
  
  switch(id)
  {
    case 1: message[1]=0x01;
            break;
    case 2: message[1]=0x02;
            break;
    case 3: message[1]=0x03;
            break;
    case 4: message[1]=0x04;
            break;
    case 5: message[1]=0x05;
            break;
    case 6: message[1]=0x06;
            break;
  }
  message[3] = smcDriver::getCRC(message, 3);
  
  smcDriver::sendCommand(message,4);
}
 
// speed should be a number from -3200 to 3200
void smcDriver::setMotorSpeed(uint8_t id, int speed)
{
  unsigned char speedmsg[6]={0xAA,0x00,0x06,0x00,0x00,0x00};
  //by default: motor reverse (0x06)
  
  switch(id)
  {
    case 1: speedmsg[1]=0x01;
            break;
    case 2: speedmsg[1]=0x02;
            break;
    case 3: speedmsg[1]=0x03;
            break;
    case 4: speedmsg[1]=0x04;
            break;
    case 5: speedmsg[1]=0x05;
            break;
    case 6: speedmsg[1]=0x06;
            break;
  }
  speed *= _thrusterRatio[id-1];
  if (speed < 0)
  {
    speed = -speed;  // make speed positive
  }
  else
  {
    speedmsg[2]=0x05; //change from reverse to forward
  }
  
  speedmsg[3]=speed&0x1F;
  speedmsg[4]=speed>>5;
  speedmsg[5]=getCRC(speedmsg,5);
  
  sendCommand(speedmsg,6);
}

void smcDriver::setThrusterRatio(float ratio[])
{
  _thrusterRatio[0]=ratio[0];
  _thrusterRatio[1]=ratio[1];
  _thrusterRatio[2]=ratio[2];
  _thrusterRatio[3]=ratio[3];
  _thrusterRatio[4]=ratio[4];
  _thrusterRatio[5]=ratio[5];
}