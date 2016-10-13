SSD1306_text -- methods
April 2015

init() - call once in setup
clear() - clears screen, sets cursor to 0,0
setCursor(row, pixel_col) - sets cursor to row (0..7) and pixel_column (0..127)
setCursor(row) - sets cursor to row (0..7) and pixel_column=0 
getRow() - returns current row
getCol() - returns current column
setTextSize(size, spacing) - sets text size (1..8) and pixel spacing between chars (0,1,2,...)
setTextSize(size) - sets text size (1..8) and sets spacing to the same value
write(c) - write a single character to the current cursor location with current size/spacing
write(*str) - write a string of characters
write(*buffer, nBytes) - write nBytes of characters from a buffer
sendData(b) - write an 8-bit vertical slice of pixels to the current cursor location

Configuration switches in the .h file provide for choosing hardware SPI, software SPI or I2C
support. Also support for scaled fonts larger than the default 5x7 (size=1) can be enabled/disabled. In addition, improved fonts for the numeric characters 0-9 for sizes 2x, 3x and 8x the 5x7 font size can be enabled/disabled.

With a minimum configuration (H/W SPI and no scaled or larger fonts) a simple "hello world" sketch is about 2500 bytes in size and uses about 50 bytes of RAM. Adding scaled fonts increases the code size somewhat. Adding dedicated larger numeric fonts has a more pronounced effect on code size.

I2C support uses the Wire library which, in addition to adding ~1K of code, adds about 200 bytes of RAM usage, mostly from the five 32-byte buffers in the library.