#include <Arduino.h>
#include "wind_sensor.h"

WindSensor::WindSensor() {
    _windDirection = 0;
    _windSpeed = 0;
    _startReading = false;
}

void WindSensor::init() {
	Serial2.begin(9600);
}

uint16_t WindSensor::getDirection() {
    return _windDirection;
}

uint16_t WindSensor::getWindSpeed() {
    return _windSpeed;
}

void WindSensor::readValues() {
    if(Serial2.available()) {
    byte input = Serial2.read();
    
    switch(input) {
      case PACKET_START:
        _startReading = true;
	_stringIndex = 0;
        break;
        
      case PACKET_END:
        _startReading = false;
        writeValues();
        _stringIndex = 0; // restart the string for next iteration
        break;

      default:
        if(!_startReading) break;
        else {
          _stringValues[_stringIndex] = (char)input;
	  _stringIndex ++;
        }
        break;
    }
  }
}

void WindSensor::writeValues() {
  _stringValues[_stringIndex] = 0x00;
  
  char * token = strtok(cstr, ",");
  for(int i = 0; i < 3; i++) {
    if(i == 1) { // the first value is the direction
      _windDirection = atoi(token);	// 0 to 359
    } else if (i == 2) { // the second value is the wind speed
      _windSpeed = atof(token)*100; // in 0.01m/s
    }
    token = strtok(NULL, ",");
  }
}


