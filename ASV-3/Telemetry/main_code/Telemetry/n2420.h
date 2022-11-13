/*
  n2420.h - Library for the new n2420 radio.
  Created by Joshua Nathanael, October 21, 2020.
  Not released into the public domain.
  Special thanks to Robin2 for the Serial Tutorial:
  https://forum.arduino.cc/index.php?topic=396450 
*/

#ifndef n2420_h
#define n2420_h

/*Use <> if globally installed, use "" if in project directory*/
#include <Arduino.h>
#include <HardwareSerial.h>

#define START_BYTE 0x7b //'{'
#define END_BYTE 0x7d   //'}'
#define ASV_EXTENSION 1
#define POKB 2
#define REMOTE_KILL 3
#define OCS_EXTENSION 4
#define RECIEVE_TIMEOUT 1000

/*
 * Send Packet Format: (For Receiving and Sending Packet)
 * 1. Start Byte
 * 2. Unit Address  [1- Master (ASV Extension), 2 - Slave (POKB), 3 - Repeater (OCS Extension), 4 - Slave (Remote Kill)]
 * 3. Operating Mode
 * 4. Message Size
 * 5. Message
 * 6. End Byte
 * So, packet = {Unit Address, Operating Mode, Message Size, Message}
 * 
 */
#define MESSAGE_BUFFER_SIZE 64
#define ESCAPE_TIMEOUT 2000

class N2420{
  private:
    uint8_t _id;
    Stream *_serial = &Serial;
    uint8_t b;
    uint32_t _start;
    uint32_t _stop;
    uint8_t _counter;
    //uint8_t _unitAddress;
    uint8_t _operatingMode;
    
    //For readPacket method
    static const byte numElements = 32;
    uint8_t receivedChars[numElements];
    //char tempChars[numElements];
    uint8_t _receiveUnitAddress;
    uint8_t _receiveOperatingMode;
    uint8_t _messageSize;
    uint8_t _messageBuf[MESSAGE_BUFFER_SIZE];
    bool newData = false;
    bool receiveTimeout;
    
    //For receiveStart method
    boolean receiveInProgress;
    byte index;
    byte loopIndex;
    char startMarker;
    char endMarker;
    char rc;

    //For parseData method
    //char * strtokIndx; // this is used by strtok() as an index
    
    //For writePacket method
    uint8_t _messageSend[MESSAGE_BUFFER_SIZE];
    
  public:
    N2420(uint8_t hardware_id);
    uint8_t getId();
    void setSerial(Stream *serialIn);
    void sendText(char *someText);
    void sendBuf(const uint8_t *buf, size_t bufSize);
    uint8_t readByte();
    bool isAvailable();
    void readPacket();
    void receiveStart();
    
    void parseData();
    uint8_t* showParsedData();
    uint8_t* showReceived();
    uint8_t getReceivingAddress();
    uint8_t getReceivingMode();
    uint8_t getMessageSize();
    void writeByte(uint8_t val);
    void writePacket(int buf[], uint8_t bufSize);

    uint8_t getCount();
    uint8_t getVal();
    void escape_mode();
    bool escape_mode_response();
    void baudRate();
    void operatingMode();
};



//class ATCommands : public N2420{
//  private:
//    HardwareSerial *_serial;
//    uint8_t b;
//  public:
//    ATCommands();
//    void Initialise();
//    //void escape_mode();
//    void operatingMode();
//    void unitAddress();
//    void destinationAddress();
//    void Roaming();
//    void networkType();
//    //void baudRate();
//    void dataFormat();
//    void FEC();
//    void packetRetransmission();
//    void doneInitialise();
//};

#endif
