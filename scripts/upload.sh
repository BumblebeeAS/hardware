#!/bin/sh  
ssh bbauvsbc1@bbauv "avrdude -p atmega2560 -b 115200 -c stk500v2 -P/dev/%1 -U flash:w:mega.hex"
