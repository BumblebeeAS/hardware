#include <Wire.h>
#define vehiclePower 6
#define boardPower 3

byte highbyteAIN0, lowbyteAIN0, highbyteAIN1, lowbyteAIN1;
uint16_t temp0=0, temp1=0;
float voltage0, voltage1;

void setup()
{
	Serial.begin(9600);
  begin_ads1115();
  pinMode(vehiclePower, OUTPUT);
  digitalWrite(vehiclePower, HIGH);
  pinMode(boardPower, OUTPUT);
  digitalWrite(boardPower, LOW);
}

void loop()
{
	read_AIN0();
    delay(5);
    read_AIN1();

    if((highbyteAIN0>>7)&0x1==1)
     temp0=0;
     else
     {
         temp0 = highbyteAIN0 * 256;
         temp0 = temp0 + lowbyteAIN0;  
     }
   voltage0=temp0;
   Serial.print("raw ADC0: ");
   Serial.println(temp0);
   voltage0=voltage0/32767*4.096/40;   
   Serial.print("voltage ADC0: ");
   Serial.println(voltage0);
    if((highbyteAIN1>>7)&0x1==1)
     temp1=0;
     else
     {
         temp1 = highbyteAIN1 * 256;
         temp1 = temp1 + lowbyteAIN1;  
     }
   voltage1=temp1;
   Serial.print("raw ADC1: ");
   Serial.println(temp1);
   voltage1=voltage1/32767*4.096/40;
   Serial.print("voltage ADC1: ");
   Serial.println(voltage1);
   Serial.println();
   delay(1000);
}

void begin_ads1115()
{
  Wire.begin();                 // join i2c bus (address optional for master)
  Wire.beginTransmission(72);   // transmit to device #72 (0b1001000)  
  Wire.write(1);                // point register set to config register
  Wire.write(194);              // configure ADS chip, this if the MSByte of the 16bit config register
  Wire.write(227);              // LSByte of the 16bit config register 
  Wire.endTransmission();       // stop transmitting   
  Wire.beginTransmission(72);   // transmit to device #72 (0b1001000)  
  Wire.write(0);                // point register set to config register
  Wire.endTransmission();       // stop transmitting 
  
}

void read_AIN0()
{
  Wire.requestFrom(72, 2);    // request 6 bytes from slave device #72 (0b1001000)
  while(Wire.available())     // slave may send less than requested
  { 
    highbyteAIN0 = Wire.read(); 
    lowbyteAIN0 = Wire.read();
  } 
  
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(1);               // point register set to config register
  Wire.write(82);              // configure ADS chip, this is the MSByte of the 16bit config register
  Wire.write(227);             // LSByte of the 16bit config register  
  Wire.endTransmission();      // stop transmitting  
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(0);               // point register set to config register  
  Wire.endTransmission();      // stop transmitting  
}

void read_AIN1()
{
  Wire.requestFrom(72, 2);    // request 6 bytes from slave device #72 (0b1001000)
  while(Wire.available())     // slave may send less than requested
  { 
    highbyteAIN1 = Wire.read(); 
    lowbyteAIN1 = Wire.read();
  } 

  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(1);               // point register set to config register
  Wire.write(66);              // configure ADS chip, this is the MSByte of the 16bit config register
  Wire.write(227);             // LSByte of the 16bit config register  
  Wire.endTransmission();      // stop transmitting
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(0);               // point register set to config register  
  Wire.endTransmission();      // stop transmitting

}
