#include "HIH613x.h"

HIH613x::HIH613x(byte address) : address(address) { }

HIH613x::~HIH613x() { }

void HIH613x::measurementRequest() const
{
    Wire.beginTransmission(address);
    Wire.endTransmission();
}

byte HIH613x::dataFetch()
{
    byte read_byte, status_data;
    uint16_t temperature_data, humidity_data;

    // request 4 bytes
    Wire.requestFrom(address, (uint8_t) 4);

    // response error
    if(!Wire.available()) {
        Wire.endTransmission();
        return 0x10;
    }

    // byte 1
    read_byte = Wire.read();
    status_data = read_byte >> 6;
    humidity_data = (read_byte & 0x3f) << 8;

    // byte 2
    read_byte = Wire.read();
    humidity_data += read_byte;

    // byte 3
    read_byte = Wire.read();
    temperature_data = read_byte << 6;

    // byte 4
    read_byte = Wire.read();
    temperature_data += read_byte >> 2;

    // end transmission
    Wire.endTransmission();

    // hih error
    if (status_data != 0) {
        return status_data;
    }

    // convert
    humidity = humidity_data / (pow(2, 14) - 1) * 100;
    temperature = temperature_data / (pow(2, 14) - 1) * 165 - 40;

    return 0;
}

byte HIH613x::update()
{
    measurementRequest();
    delay(50);
    return dataFetch();
    
}
