#include <Arduino.h>
#include "define.h"
#include "wind_sensor.h"

WindSensor::WindSensor() {
    _windDirection = 0;
    _windSpeed = 0;
    _startReading = false;
    _stringValues = "";
}

double WindSensor::getDirection() {
    return _windDirection;
}

double WindSensor::getWindSpeed() {
    return _windSpeed;
}

void WindSensor::readValues() {
    if(Serial1.available()) {
    byte input = Serial1.read();
    
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
      _windDirection = atof(token);
    } else if (i == 2) { // the second value is the wind speed
      _windSpeed = atof(token);
    }
    token = strtok(NULL, ",");
  }
}


