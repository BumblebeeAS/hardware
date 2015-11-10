echo off
pscp -pw password C:\Users\gohew\AppData\Local\V.Micro\Arduino\Builds\can_send_bandwidth\uno\can_send_bandwidth.hex gohew@192.168.1.34:mcu.hex

if "%1"=="mega2560" plink -pw password gohew@192.168.1.34 avrdude -p atmega2560 -b 115200 -c stk500v2 -P/dev/%2 -U flash:w:mcu.hex

if "%1"=="mega328p" plink -pw password gohew@192.168.1.34 avrdude -p atmega328p -b 115200 -c stk500v2 -P/dev/%2 -U flash:w:mcu.hex


