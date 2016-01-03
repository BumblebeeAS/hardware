#include <SPI.h>

#include <Wire.h>
/*REMEMBER
SDA = A4, SCL = A5 FOR BOARD TESTING
SDA = 20, SCL = 21 FOR TESTING ON MEGA
*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TempAD7414.h>
#include <EEPROMPlus.h>


//OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
//*OLED

//EEPROM
#define disk1 0x50
EEPROMPlus mem = EEPROMPlus(disk1);
//*EEPROM

//Temperature Sensor
TempAD7414 temp_sens(96);
//*Tesmperature Sensor

//Pressure Sensor
//*Pressure Sensor

//ADS
//*ADS

void setup()   {                
  Serial.begin(9600);
  //Scan I2C bus
  Wire.begin();
  //*Scan I2C bus
  
  //OLED
  // initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D); 
  display.clearDisplay();
  //*OLED

  //EEPROM
  //*EEPROM

  //Temperature Sensor
  temp_sens.initTempAD7414();
  //*Tesmperature Sensor

  //Pressure Sensor
  //*Pressure Sensor

  //ADS
  //*ADS
}


void loop() {
  //OLED
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25,0);
  display.println("Testing OLED...");
  display.display();
  delay(2000);
  display.setCursor(50,25);
  display.println("SUCK MY DICK!!");
  display.display();
  delay(2000);
  display.clearDisplay();
  //*OLED

  //I2C Scanner  
  Serial.println("\nI2C Scanner");
  I2CScan();
  //*I2C Scanner

  //EEPROM
  Serial.println("EEPROM");
  mem.dumbWrite(0, 123);
  byte data = mem.dumbRead(0);
  Serial.println(data, DEC);
  //*EEPROM

  //Temperature Sensor
  Serial.println("Temperature");
  Serial.print(temp_sens.getTemp());
  //*Tesmperature Sensor

  //Pressure Sensor
  Serial.println("Pressure");
  //*Pressure Sensor

  //ADS
  Serial.println("ADS RAW");
  
  //*ADS


  //finish
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(25,0);
  display.println("Finished Tests!!!");
  display.display();
  while(true){
  	//do nothing
  }
  //*finish
}

void I2CScan(){
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Unknow error at address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
