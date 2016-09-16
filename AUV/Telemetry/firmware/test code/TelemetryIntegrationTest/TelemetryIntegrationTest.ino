/******************************************************************
 This is an example for the Adafruit RA8875 Driver board for TFT displays
 ---------------> http://www.adafruit.com/products/1590
 The RA8875 is a TFT driver for up to 800x480 dotclock'd displays
 It is tested to work with displays in the Adafruit shop. Other displays
 may need timing adjustments and are not guanteed to work.
 
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 ******************************************************************/

#include <SD.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

File myFile;

// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 3
#define RA8875_CS 7
#define RA8875_RESET 9

const int chipSelect = 10;

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;
int test;
char string[200] = "Lorem ipsum dolor sit amet, consectetur adipiscing";
int i = 0;
unsigned long time = millis();
boolean red;

char u;

void setup() 
{
  Serial.begin(9600);
  Serial2.begin(9600);
  
  Serial.println("RA8875 start");

  /* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);
  tft.fillScreen(RA8875_CYAN);

  /* Switch to text mode */  
  tft.textMode();

  /* Change the cursor location and color ... */  
  tft.textSetCursor(100, 100);
  tft.textTransparent(RA8875_YELLOW);

  /* ... and render some more text! */
  tft.textWrite(string);
  
  
  
  Serial.println("Screen finish");
  pinMode(SS, OUTPUT);
  
  
  

}

void loop() 
{
  
  
  tft.textMode();

  if (test > 480){
    test = 0;
    tft.writeReg(0x8E, 0x80 | 0x00);
    delay(30);
  }
  
  
 
  
  SD.begin(chipSelect);
  myFile = SD.open("test.txt", FILE_WRITE);
  myFile.write("testing 1, 2, 3.\n");
  // close the file:
  myFile.close();
  
  if (Serial2.available()){
    u = Serial2.read();
    
    if(u == 'S'){
      tft.writeReg(0x8E, 0x80 | 0x00);
    }else if(u == 'L'){
      char string2[20] = "LEFT";
      tft.textSetCursor(300, 200);
      tft.textTransparent(RA8875_YELLOW);
      tft.textEnlarge(4);
      tft.textWrite(string2);
      //Serial.println("LEFT");
    }else if(u == 'F'){
      char string3[20] = "FORWARD";
      tft.textSetCursor(300, 200);
      tft.textTransparent(RA8875_YELLOW);
      tft.textEnlarge(4);
      tft.textWrite(string3);
      //Serial.println("FORWARD");
    }else if(u == 'B'){
      char string4[20] = "BACKWARD";
      tft.textSetCursor(300, 200);
      tft.textTransparent(RA8875_YELLOW);
      tft.textEnlarge(4);
      tft.textWrite(string4);
      //Serial.println("BACKWARD");
    }else if(u == 'R'){
      char string1[20] = "RIGHT";
      tft.textSetCursor(300, 200);
      tft.textTransparent(RA8875_YELLOW);
      tft.textEnlarge(4);
      tft.textWrite(string1);
      //Serial.println("RIGHT");
    }
  }
  
}
