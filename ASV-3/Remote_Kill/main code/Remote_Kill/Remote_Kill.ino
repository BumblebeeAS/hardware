#include "n2420.h"
#include <SoftwareSerial.h>
#include "defines.h"

// create the N2420 object
N2420 n2420 (REMOTE_KILL);
int payload[] = { 0 };

static uint32_t xbee_loop = 0;
int8_t sentByte = 0x15;
uint8_t inByte;

SoftwareSerial mySerial(6, 7); // RX, TX

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //actual baud rate is /2 which is 115200
  mySerial.begin(115200); //actual baud rate is /2 which is 115200
  //xbee.setSerial(mySerial);
  n2420.setSerial(&mySerial);

  pinMode(3, INPUT);
  xbee_loop = millis();

  Serial.println("Restart");
}

void loop() {

  if ((millis() - xbee_loop) > 500){
  
    if (digitalRead(3)== HIGH){
      sentByte = 0x15;
      Serial.println("normal");       //Normal Operations when HIGH
    }
    else{
      sentByte = 0x44;                
      Serial.println("KILL");         //KILL Operations when LOW
    }
    
    Serial.print("sendbyte: ");
    Serial.println(sentByte, HEX);
    payload[0] = sentByte;
    Serial.print("payload: ");
    Serial.println(payload[0], HEX);
    n2420.writePacket(payload,1);
    xbee_loop = millis();
    //batteryCheck();
  }

}
