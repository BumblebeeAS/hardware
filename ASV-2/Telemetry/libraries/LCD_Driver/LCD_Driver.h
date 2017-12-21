#ifndef _LCD_
#define _LCD_

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_RA8875.h>  //screen

#ifdef _SD_
#include <SD.h>  //SD card
#endif

class LCD
{
private:
	Adafruit_RA8875* screen;
	int _x;
	int _y;

#ifdef _SD_
	// Draw bmp img
	uint16_t read16(File f);
	uint32_t read32(File f);
	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
	byte decToBcd(byte val);
#endif

	void increment_row();

public:
	LCD(int screen_cs, int screen_reset);
	//~LCD();
	void screen_init();
	
	void set_cursor(uint32_t x, uint32_t y);
	
	void write_string(const char* var);
	void write_value_int(uint32_t var);
	void write_value_string(const char* var);

	void screen_clear();
	
#ifdef _SD_
	void bmpDraw(char *filename, int picx, int picy);
#endif

};

#endif