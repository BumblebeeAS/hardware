###echo off
pscp -pw bb %3 bbauvsbc1@192.168.1.130:mcu.hex

if "%1"=="mega2560" plink -pw bb bbauvsbc1@192.168.1.130 avrdude -C "/etc/avrdude.conf" -v -patmega2560 -cwiring -P/dev/%2 -b115200 -D -Uflash:w:mcu.hex:i

if "%1"=="mega328p" plink -pw bb bbauvsbc1@192.168.1.130 avrdude -p atmega328p -b 115200 -c stk500v2 -P/dev/%2 -U flash:w:mcu.hex

