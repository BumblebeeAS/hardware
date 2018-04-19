#include <Arduino.h>
#include "LCD_Driver.h"
#include <stdio.h>

LCD::LCD(int screen_cs, int screen_reset)
{
	screen = new Adafruit_RA8875(screen_cs, screen_reset);  //screen
	_x = 0;
	_y = 0;
}

void LCD::screen_init(){
	/* Initialise the display using 'RA8875_480x272' or 'RA8875_800x480' */
	if (!screen->begin(RA8875_800x480)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	screen->displayOn(true);
	screen->GPIOX(true);      // Enable screen - display enable tied to GPIOX
	screen->PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	screen->PWM1out(255);
	screen->fillScreen(RA8875_GREEN);  //Fill screen with cyan color
	delay(100);
	screen->fillScreen(RA8875_CYAN);
	delay(100);
	screen->fillScreen(RA8875_BLUE);
	delay(100);
	screen->fillScreen(RA8875_RED);
	delay(100);
	screen->fillScreen(RA8875_RED);
	delay(100);
	screen->fillScreen(RA8875_MAGENTA);
	delay(100);
	screen->fillScreen(RA8875_YELLOW);
	delay(100);
	screen->fillScreen(RA8875_BLACK);

	/* Switch to graphic mode */  
	screen->graphicsMode();                 // go back to graphics mode
	screen->fillScreen(RA8875_BLACK);
#ifdef _SD_
	screen->graphicsMode();   
	bmpDraw("bbauv.bmp", 0, 0);
#endif
	screen->fillScreen(RA8875_BLACK);
	screen->textEnlarge(1);
	screen->textMode();
}

void LCD::screen_clear(){
	screen->writeReg(0x8E, 0x80 | 0x00);  //clear screen
}

void LCD::set_cursor(uint32_t x, uint32_t y)
{
	_x = x;
	_y = y;
	screen->textSetCursor(x, y);
}
void LCD::increment_row()
{
	_y += 35;
	screen->textSetCursor(_x, _y);
}

//============================
//		Write to screen
//============================

static char dummyChar[9];
static String dummyStr;

void LCD::write_string(const char* var){
	screen->textTransparent(RA8875_WHITE);
	screen->textWrite(var);
	increment_row();
}

void LCD::write_value_int(uint32_t var){
	screen->fillRect(_x, _y, 130, 30, RA8875_BLACK);
	screen->textTransparent(RA8875_YELLOW);
	if (var == 255){
		write_value_string("N/A");
	}else{
		char buf[20] = {};
		sprintf(buf, "%lu", var);
		screen->textWrite(buf);
	increment_row();
	}
}

void LCD::write_value_string(const char* var){
	if(var == "YES"){
		screen->fillRect(_x, _y, 50, 30, RA8875_GREEN);
		screen->textTransparent(RA8875_BLACK);
	}
	else if(var == "NO"){
		screen->fillRect(_x, _y, 50, 30, RA8875_RED);
		screen->textTransparent(RA8875_WHITE);
	}
	else{
		screen->fillRect(_x, _y, 50, 30, RA8875_BLACK);
		screen->textTransparent(RA8875_YELLOW);
	}
	screen->textWrite(var);
	increment_row();
}


#ifdef _SD_

//=======================
//		BMP
//=======================

#define BUFFPIXEL 75

void LCD::bmpDraw(char *filename, int picx, int picy) {
	File     bmpFile;
	int      bmpWidth, bmpHeight;   // W+H in pixels
	uint8_t  bmpDepth;              // Bit depth (currently must be 24)
	uint32_t bmpImageoffset;        // Start of image data in file
	uint32_t rowSize;               // Not always = bmpWidth; may have padding
	uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
	uint16_t lcdbuffer[BUFFPIXEL];  // pixel out buffer (16-bit per pixel)
	uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
	boolean  goodBmp = false;       // Set to true on valid header parse
	boolean  flip    = true;        // BMP is stored bottom-to-top
	int      w, h, row, col;
	uint8_t  r, g, b;
	uint32_t pos = 0, startTime = millis();
	uint8_t  lcdidx = 0;
	boolean  first = true;

	if((picx >= screen->width()) || (picy >= screen->height())) return;

	Serial.println();
	Serial.print(F("Loading image '"));
	Serial.print(filename);
	Serial.println('\'');

	// Open requested file on SD card
	if ((bmpFile = SD.open(filename)) == NULL) {
		Serial.println(F("File not found"));
		return;
	}

	// Parse BMP header
	if(read16(bmpFile) == 0x4D42) { // BMP signature
		Serial.println(F("File size: ")); 
		Serial.println(read32(bmpFile));
		(void)read32(bmpFile); // Read & ignore creator bytes
		bmpImageoffset = read32(bmpFile); // Start of image data
		Serial.print(F("Image Offset: ")); 
		Serial.println(bmpImageoffset, DEC);

		// Read DIB header
		Serial.print(F("Header size: ")); 
		Serial.println(read32(bmpFile));
		bmpWidth  = read32(bmpFile);
		bmpHeight = read32(bmpFile);

		if(read16(bmpFile) == 1) { // # planes -- must be '1'
			bmpDepth = read16(bmpFile); // bits per pixel
			Serial.print(F("Bit Depth: ")); 
			Serial.println(bmpDepth);
			if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
				goodBmp = true; // Supported BMP format -- proceed!
				Serial.print(F("Image size: "));
				Serial.print(bmpWidth);
				Serial.print('x');
				Serial.println(bmpHeight);

				// BMP rows are padded (if needed) to 4-byte boundary
				rowSize = (bmpWidth * 3 + 3) & ~3;

				// If bmpHeight is negative, image is in top-down order.
				// This is not canon but has been observed in the wild.
				if(bmpHeight < 0) {
					bmpHeight = -bmpHeight;
					flip      = false;
				}

				// Crop area to be loaded
				w = bmpWidth;
				h = bmpHeight;
				if((picx+w-1) >= screen->width())  w = screen->width()  - picx;
				if((picy+h-1) >= screen->height()) h = screen->height() - picy;

				// Set screen address window to clipped image bounds

				for (row=0; row<h; row++) { // For each scanline...
					// Seek to start of scan line.  It might seem labor-
					// intensive to be doing this on every line, but this
					// method covers a lot of gritty details like cropping
					// and scanline padding.  Also, the seek only takes
					// place if the file position actually needs to change
					// (avoids a lot of cluster math in SD library).
					if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
						pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
					else     // Bitmap is stored top-to-bottom
						pos = bmpImageoffset + row * rowSize;
					if(bmpFile.position() != pos) { // Need seek?
						bmpFile.seek(pos);
						buffidx = sizeof(sdbuffer); // Force buffer reload
					}

					for (col=0; col<w; col++) { // For each column...
						// Time to read more pixel data?
						if (buffidx >= sizeof(sdbuffer)) { // Indeed
							// Push LCD buffer to the display first
							if(lcdidx > 0) {
								screen->drawPixel(col+picx, row+picy, lcdbuffer[lcdidx]);
								lcdidx = 0;
								first  = false;
							}

							bmpFile.read(sdbuffer, sizeof(sdbuffer));
							buffidx = 0; // Set index to beginning
						}

						// Convert pixel from BMP to screen format
						b = sdbuffer[buffidx++];
						g = sdbuffer[buffidx++];
						r = sdbuffer[buffidx++];
						lcdbuffer[lcdidx] = color565(r,g,b);
						screen->drawPixel(col+picx, row+picy, lcdbuffer[lcdidx]);
					} // end pixel

				} // end scanline

				// Write any remaining data to LCD
				if(lcdidx > 0) {
					screen->drawPixel(col+picx, row+picy, lcdbuffer[lcdidx]);
				} 

				Serial.print(F("Loaded in "));
				Serial.print(millis() - startTime);
				Serial.println(" ms");

			} // end goodBmp
		}
	}

	bmpFile.close();
	if(!goodBmp) Serial.println(F("BMP format not recognized."));

}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t LCD::read16(File f) {
	uint16_t result;
	((uint8_t *)&result)[0] = f.read(); // LSB
	((uint8_t *)&result)[1] = f.read(); // MSB
	return result;
}

uint32_t LCD::read32(File f) {
	uint32_t result;
	((uint8_t *)&result)[0] = f.read(); // LSB
	((uint8_t *)&result)[1] = f.read();
	((uint8_t *)&result)[2] = f.read();
	((uint8_t *)&result)[3] = f.read(); // MSB
	return result;
}

uint16_t LCD::color565(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

byte LCD::decToBcd(byte val){
	// Convert normal decimal numbers to binary coded decimal
	return ( (val/10*16) + (val%10) );
}

#endif