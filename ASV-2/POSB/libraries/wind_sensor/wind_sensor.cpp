#include <Arduino.h>
#include "define.h"
#include "wind_sensor.h"

WindSensor::WindSensor() {
    _windDirection = 0;
    _windSpeed = 0;
    _startReading = false;
    _stringValues = "";
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
        break;
        
      case PACKET_END:
        _startReading = false;
        writeValues();
        _stringValues = ""; // restart the string for next iteration
        break;

      default:
        if(!_startReading) break;
        else {
          _stringValues += (char)input;
        }
        break;
    }
  }
}

void WindSensor::writeValues() {
  char * cstr = new char[_stringValues.length() + 1];
  strcpy(cstr, _stringValues.c_str());
  
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


