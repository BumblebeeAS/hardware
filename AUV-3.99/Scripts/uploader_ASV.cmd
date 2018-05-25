###echo off
echo "%1 = mega2560 or mega328p, %2 = port (ttySA), %3 = file directory"
pscp -pw bb %3 bbauv@192.168.1.130:mcu.hex

if "%1"=="mega2560" plink -pw bb bbauv@192.168.1.130 avrdude -C "/etc/avrdude.conf" -v -patmega2560 -cwiring -P/dev/%2 -b115200 -D -Uflash:w:mcu.hex:i

if "%1"=="mega328p" plink -pw bb bbauv@192.168.1.130 avrdude -p atmega328p -b 57600 -carduino -P/dev/%2 -D -V -Uflash:w:mcu.hex:i

