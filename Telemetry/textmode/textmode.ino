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


#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 3
#define RA8875_CS 10
#define RA8875_RESET 9

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;
int test;
char string[200] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam bibendum lacinia diam vitae porttito";
int i = 0;
unsigned long time = millis();
boolean red;

void setup() 
{
  Serial.begin(9600);
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
  
  

}

void loop() 
{
  uint8_t cmd;
  
  if (test > 480){
    test = 0;
    tft.writeReg(0x8E, 0x80 | 0x00);
    delay(30);
  }
  /*
  if (i > 1000){
    i = 0;
    Serial.println(millis()-time);
    time = millis();
  }
  
  tft.textSetCursor(0, test);
  tft.textWrite(string);
  i++;
  delay(30);
  */
  
  if (millis() - time > 200){
    /*
    time = millis();
    tft.textSetCursor(0, test);
    tft.textWrite(string);
    test += 15;
    */
    tft.fillRect(0, test, 800, 10, RA8875_BLUE);
    test += 10;
  }
  
  
  Serial.println(millis()-time);
  
  
}
