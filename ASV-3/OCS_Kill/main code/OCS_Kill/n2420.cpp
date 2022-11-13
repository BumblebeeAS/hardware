/*
  n2420cpp - Library for the new radio.
  Created by Joshua Nathanael, October 21, 2020.
  Not released into the public domain.
  Special thanks to Robin2 for the Serial Tutorial:
  https://forum.arduino.cc/index.php?topic=396450 
*/

#include <Arduino.h>
#include "n2420.h"

#include <HardwareSerial.h>


N2420::N2420(uint8_t hardware_id){

  _id = hardware_id;
  _serial = NULL;
  b = 0;
  _start = 0;
  _stop = 0;
  _counter = 0;
  //For receiveStart
  receiveInProgress = false;
  index = 0;
  loopIndex = 0;
  startMarker = START_BYTE;
  endMarker = END_BYTE;
  
}
uint8_t N2420::getId(){
  return _id;
}
void N2420::setSerial(Stream *serialIn){
  _serial = serialIn;
}
void N2420::sendText(char *someText){
  _serial -> write(someText);
}
void N2420::sendBuf(const uint8_t *buf, size_t bufSize){
  _serial -> write(buf, bufSize);
}
uint8_t N2420::readByte(){
  return _serial->read();
}
bool N2420::isAvailable(){
  return _serial->available();
}

void N2420::readPacket(){
  N2420::receiveStart();
  if (newData){
    parseData();
    newData = false;  
  }
}
void N2420::receiveStart(){
  _start = millis();
  while ((N2420::isAvailable()) && newData == false) {
        rc = N2420::readByte();

//        if ((millis() - _start) > ESCAPE_TIMEOUT){
//          return 1;
//        }
        
        if (receiveInProgress == true) {
          _start = millis();
          if (rc != endMarker) {
              receivedChars[index] = rc;
              index++;
              if (index >= numElements) {
                  index = numElements - 1;  //this will replace the last element when exceeded
              }
          }
          else {
              receivedChars[index] = '\0'; // terminate the string
              receiveInProgress = false;
              loopIndex = index;
              index = 0;
              newData = true;
              //return 0;
          }
        }

        else if (rc == startMarker) {
            receiveInProgress = true;
            _start = millis();
        }

        if (millis() - _start > RECIEVE_TIMEOUT){
          break;  
        }

    }
}
void N2420::parseData(){
  _receiveUnitAddress = receivedChars[0];
  _operatingMode = receivedChars[1];
  _messageSize = receivedChars[2];
  for (int j = 3; j < loopIndex; j++){
      _messageBuf[j-3] = receivedChars[j];
  }
}
uint8_t* N2420::showParsedData(){
  return _messageBuf;
}
uint8_t* N2420::showReceived(){
  return receivedChars;
}
uint8_t N2420::getReceivingAddress(){
  return _receiveUnitAddress;
}
uint8_t N2420::getReceivingMode(){
  return _receiveOperatingMode;
}
uint8_t N2420::getMessageSize(){
  return _messageSize;
}
void N2420::writeByte(uint8_t val){
  _serial->write(val);
}
void N2420::writePacket(int buf[], uint8_t bufSize){
  _messageSend[0] = startMarker;
  _messageSend[1] = N2420::getId();
  _messageSend[2] = bufSize;
  for (int i = 3; i < bufSize + 3; i++){
     _messageSend[i] = (uint8_t) buf[i-3];
  }
  _messageSend[bufSize+3] = endMarker;
  N2420::sendBuf(_messageSend, sizeof(_messageSend));
}

uint8_t N2420::getVal(){
  return b;
}
uint8_t N2420::getCount(){
  return _counter;
}
void N2420::escape_mode(){
  N2420::sendText((char *)"+++");
}
bool N2420::escape_mode_response(){
  if (N2420::isAvailable()){
    b = readByte();
    if (b == 43){
      _counter++;
    }
  }
  if (_counter == 3){
    return true;
  }
}
void N2420::baudRate(){
  N2420::sendText((char *)"ATS102?\r");
  if (N2420::isAvailable()){
    b = N2420::readByte();
  }
}

void N2420::operatingMode(){
  _serial->write("ATS101?\r");
  if (isAvailable()){
    b = readByte(); 
  }
}

//ATCommands::ATCommands(){
//  //Serial.println("ATCommand Object Created");
//}

//void N2420::escape_mode(){
//  N2420::sendText("+++"); 
//}

//void ATCommands::operatingMode(){
//  _serial->write("ATS101?\r");
//  if (isAvailable()){
//    b = readByte(); 
//    if (b == 0){
//      Serial.print("Master Unit");
//    }
//    else if (b == 1){
//      Serial.print("Repeater Unit");
//    }
//    else{
//      Serial.print("Slave Unit");
//    }
//  }
//}
//void ATCommands::unitAddress(){
//  _serial->write("ATS105?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::destinationAddress(){
//  _serial->write("ATS140?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::Roaming(){
//  _serial->write("ATS118?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::networkType(){
//  _serial->write("ATS133?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void N2420::baudRate(){
//  sendText("ATS102?\r");
//  if (N2420::isAvailable()){
//    b = N2420::readByte();
//    //Serial.println(b);
//  }
//}
//void ATCommands::dataFormat(){
//  _serial->write("ATS110?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::FEC(){
//  _serial->write("ATS158?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::packetRetransmission(){
//  _serial->write("ATS113?\r");
//  if (isAvailable()){
//    b = readByte();
//    Serial.println(b);
//  }
//}
//void ATCommands::doneInitialise(){
//  _serial->write("ATA\r");
//  Serial.println("Done");
//}
//
//
//void ATCommands::Initialise(){
//  _serial = N2420::getSerial();
//  escape_mode();
//  operatingMode();
//  unitAddress();
//  destinationAddress();
//  Roaming();
//  networkType();
//  baudRate();
//  dataFormat();
//  FEC();
//  packetRetransmission();
//  doneInitialise();
//}
