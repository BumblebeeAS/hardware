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

void batteryCheck() {
  double batteryBit = analogRead(BATTERY_READ);
  double batteryVoltage = (batteryBit - 13.3) / 227.5;
  Serial.print("Battery voltage: ");
  Serial.println(batteryVoltage);

  if(batteryVoltage < 3.8) {
    digitalWrite(LED_LOW_BATT, HIGH);
  } else {
    digitalWrite(LED_LOW_BATT, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400); //actual baud rate is /2 which is 115200
  mySerial.begin(230400); //actual baud rate is /2 which is 115200
  //xbee.setSerial(mySerial);
  n2420.setSerial(&mySerial);

  pinMode(3, INPUT);
  xbee_loop = millis();

  pinMode(BATTERY_READ, INPUT);
  pinMode(LED_LOW_BATT, OUTPUT);
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
