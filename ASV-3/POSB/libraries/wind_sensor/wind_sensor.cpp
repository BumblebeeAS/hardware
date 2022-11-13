#include <Arduino.h>
#include "wind_sensor.h"

WindSensor::WindSensor() {
    _windDirection = 0;
    _windSpeed = 0;
    _startReading = false;
}

void WindSensor::init() {
	Serial2.begin(2400);
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
        Serial.println("");
        _startReading = false;
        writeValues();
        _stringIndex = 0; // restart the string for next iteration
        break;

      default:
        if(!_startReading) break;
        // else if(_stringIndex >= 20){
        //   Serial.print("Trying to catch PACKET_START\n");
        //   _stringIndex++;
        //   Serial.print(_stringIndex);
        //   Serial.print(": ");
        //   Serial.print((char) input);
        //   Serial.println("");
        //   break;
        // }
        else {
          // Serial.println("");
          // Serial.print(_stringIndex);
          // Serial.print(": ");
          // Serial.print((char) input);
          // Serial.print(" (");
          // Serial.print((int) input);
          // Serial.print(" ");
          _stringValues[_stringIndex] = (char)input;
	        _stringIndex ++;
        }
        break;
    }
  }
}

void WindSensor::writeValues() {
  _stringValues[_stringIndex] = 0x00;
  // Serial.print("complete list: ");
  // for (int i = 0; i < _stringIndex; i++){
  //   Serial.print((char) _stringValues[i]);
  //   Serial.print(" ");
  // }
  // Serial.println("");
  char * token = strtok(_stringValues, ",");
  for(int i = 0; i < 3; i++) {
    if(i == 1) { // the first value is the direction
      _windDirection = atoi(token);	// 0 to 359
      // Serial.print("token1: ");
      // Serial.println(token);
    } else if (i == 2) { // the second value is the wind speed
      _windSpeed = atof(token)*100; // in 0.01m/s
      // Serial.print("token2: ");
      // Serial.println(token);
    }
    token = strtok(NULL, ",");
  }
}


