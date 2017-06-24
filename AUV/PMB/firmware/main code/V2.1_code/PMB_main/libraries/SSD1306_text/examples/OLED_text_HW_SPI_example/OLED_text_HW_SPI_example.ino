// Simple Hello world demo using H/W SPI
#include <SPI.h>
#include <SSD1306_text.h>

// Hardware SPI pins include D11=Data and D13=Clk
#define OLED_DC 5
#define OLED_CS 6
#define OLED_RST 9
SSD1306_text oled(OLED_DC, OLED_RST, OLED_CS);

//------------------------------------------------------------------------------
void setup() {
  
// Initialize, optionally clear the screen
    oled.init();
    oled.clear();                 // clear screen
    
// Hello world - single sized character at row 0, pixel 0    
    oled.write("Hello world!");

// Scaled characters, extra spacing
    oled.setCursor(3, 10);        // move cursor to row 3, pixel column 10
    oled.setTextSize(3, 8);       // 3X character size, spacing 8 pixels
    oled.write("Abc");

// Use print()
    float floatVal = 23.792;
    oled.setCursor(6,40);
    oled.setTextSize(2,1);
    oled.print(floatVal,3);
    
// Pseudo-graphics: Draw a box using direct writes  
    oled.setCursor(0, 100);
    oled.sendData(0xFF);
    for (int i=0; i<14; i++) oled.sendData(0x01);
    oled.sendData(0xFF);
    oled.setCursor(1,100);
    oled.sendData(0xFF);
    for (int i=0; i<14; i++) oled.sendData(0x80);
    oled.sendData(0xFF);
}
//------------------------------------------------------------------------------

void loop() {}
