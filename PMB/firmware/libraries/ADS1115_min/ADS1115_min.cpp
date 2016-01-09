//ADS1115_min.cpp

#include <ADS1115_min.h>

ADS1115::ADS1115(int i2c_address){
	address = uint8_t(i2c_address);
}

void ADS1115::init(){
	Wire.begin();                 // join i2c bus (address optional for master)
	Wire.beginTransmission(address);   // transmit to device #72 (0b1001000)  
	Wire.write(1);                // point register set to config register
	Wire.write(194);              // configure ADS chip, this if the MSByte of the 16bit config register
	Wire.write(227);              // LSByte of the 16bit config register 
	Wire.endTransmission();       // stop transmitting   
}

uint16_t ADS1115::readChannel(uint8_t input){
	int config_high_byte = 0;
	byte high_byte = 0;
	byte low_byte = 0;
	uint16_t combined = 0;
	switch(input){
		case 0:
		config_high_byte = 82;	//01000010
		break;
		case 1:
		config_high_byte = 98;	//‭01010010‬
		break;
		case 2:
		config_high_byte = 114;	//‭01100010‬
		break;
		case 3:
		config_high_byte = 66;	//‭01110010‬
		break;
	}
	Wire.beginTransmission(address);  // transmit to device #72 (0b1001000)  
	Wire.write(1);               // point register set to config register
	Wire.write(config_high_byte);// configure ADS chip, this is the MSByte of the 16bit config register
	Wire.write(227);             // LSByte of the 16bit config register  
	Wire.endTransmission();      // stop transmitting
	Wire.beginTransmission(address);  // transmit to device #72 (0b1001000)  
	Wire.write(0);               // point register set to conversion register  
	Wire.endTransmission();      // stop transmitting
	Wire.requestFrom(address, 2);     // request 6 bytes from slave device #72 (0b1001000)
	while(Wire.available()){     // slave may send less than requested 
		high_byte = Wire.read(); 
		low_byte = Wire.read();
	}
	if((high_byte>>7)&0x1==1)
     combined=0;
     else
     {
         combined = high_byte * 256;
         combined = combined + low_byte;  
     }
     return combined;
}