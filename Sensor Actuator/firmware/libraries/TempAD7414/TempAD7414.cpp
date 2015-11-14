/*********************************************************************************************

SDA Pin A4 for Arduino UNO
SCL Pin A5 for Arduino UNO
AS  Pin Floating         Address 72
AS  Pin Connected -> GND Address 73 (Currently configured to this)
AS  Pin Connected -> VDD Address 74
**********************************************************************************************/

#include <TempAD7414.h>


TempAD7414::TempAD7414(int fig)
{
   configRegister = fig;
}

void TempAD7414::initTempAD7414()
{
  Wire.begin();
  Wire.beginTransmission(73);         // device address AD7414-0 AS pin connects to GND (0b 1001001)\
  Wire.write(1);              	      // device configuration register 1\
  Wire.write(configRegister);               
                                      /*D7 Full power-down if = 1. \
                             				  D6 Bypass SDA and SCL filtering if = 0. \
                              				D5 Disable ALERT if = 1. \
		                               		D4 ALERT is active low if D4 = 0, ALERT is active high if D4 = 1. \
                 		              		D3 Reset the ALERT pin if set to 1. The next temperature conversion has the ability to activate the ALERT function. The bit status is not stored; thus this bit is 0 if read. \
		                               		D2 Initiate a one shot temperature conversion if set to a 1. \
                 		              		The bit status is not stored; thus this bit is 0 if read.\
                               				(0b 01000000) default\
		                              		0b 01100000 (disable alert)\
                 		              		*/
  Wire.endTransmission();

 //Can set these registers to activate alert\
 //Alert currently disabled\
 //AD7414 THIGH REGISTER (ADDRESS 0X02) \
 //AD7414 TLOW REGISTER (ADDRESS 0X03)\
 
  Wire.beginTransmission(73);
  Wire.write(0);             			          //Temperature value register (read-only) (0b 00000000)\
  Wire.endTransmission();
}


double TempAD7414::getTemp (void)
{
  Serial.begin(9600);
    Wire.requestFrom(73,2,1);                // Requesting two bytes
    while(Wire.available())
        {
         highByteIn = Wire.read();           //D15 D14 D13 D12 D11 D10 D9 D8 
                                    	       //MSB B8  B7  B6  B5  B4  B3 B2 

         lowByteIn  = Wire.read();           //D7 D6  D5         D4         D3        D2 D1 D0 
                                             //B1 LSB ALERT_Flag THIGH_Flag TLOW_Flag 0  0  0 
        }

  temp = (highByteIn * 256);                 //Computation from the datasheet
  temp = temp + lowByteIn;

  if((temp>>15))                             //Computation for temperature below 0 degree celsius 
      {
        temp=temp>>6;
        final = (temp-512)/4.0;
      }
  
  if(!(temp>>15))                            //Computation for temperature below 0 degree celsius 
      { 
         temp=(temp>>6);
         final = temp/4.0;
      }
  


  return final;

}


