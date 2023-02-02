#include "n2420.h"

// create the N2420 object
N2420 n2420 (OCS_EXTENSION);
int payload[] = { 0 };

int8_t sentByte = 0x15;
uint8_t inByte;
static uint32_t radio_loop = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  Serial1.begin(115200); 
  n2420.setSerial(&Serial1);

  pinMode(3, INPUT);
  Serial.println("Restart");
}

void loop() {

  if ((millis() - radio_loop) > 300){
  
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
    radio_loop = millis();
  }

}
