#include <SoftwareSerial.h>
#include <XBee.h>

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response   = XBeeResponse();
ZBRxResponse rx  = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

uint8_t incomingByte;
int hardware_kill;
int no_data;
int software_kill;

SoftwareSerial mySerial(3, 2); // RX, TX

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
mySerial.begin(9600);
xbee.setSerial(mySerial);

incomingByte = 3;
hardware_kill = 1;
software_kill = 1;
no_data = 0;
pinMode(7, INPUT);
pinMode(9, OUTPUT);
pinMode(10, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(7) == HIGH){
    hardware_kill = 1;
  }
  else if (digitalRead(7) == LOW){
    hardware_kill = 0; 
  }
  Serial.print("hardkill status: ");
  Serial.println(hardware_kill);

  // read the incoming byte:
  //if (mySerial.available()){
    
    //incomingByte = mySerial.read();
  xbee.readPacket();

     if (xbee.getResponse().isAvailable())
    {
      // got something
      Serial.println("Available");
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) 
      {
        // got a zb rx packet
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(rx);
        Serial.println("Response");
        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) 
        {
           // the sender got an ACK
           //flashLed(statusLed, 5, 10);
           Serial.println("Ack"); 
        } 
        else 
        {
           // we got it (obviously) but sender didn't get an ACK
           //flashLed(errorLed, 2, 50);
           Serial.println("Send nvr get Ack");
        }
        // get data
        incomingByte = rx.getData(0);
        Serial.print("Incoming Byte: ");
        Serial.println(incomingByte, HEX); 
      } 
      else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE)
      {
        Serial.println("here?");
        xbee.getResponse().getModemStatusResponse(msr);
        // the local XBee sends this response on certain events, 
        // like association/dissociation
        
        if (msr.getStatus() == ASSOCIATED) 
        {
          // yay this is great.  flash led
          //flashLed(statusLed, 5, 100);
        } 
        else if (msr.getStatus() == DISASSOCIATED) 
        {
          // this is awful.. flash led to show our discontent
          //flashLed(errorLed, 10, 50);
        }
        else 
        {
          // another status
          //flashLed(statusLed, 10, 100);
        }
      }
    }
      else
      {
      no_data++;
      Serial.println(no_data);
        if(no_data == 20){
          software_kill = 0;
          no_data = 0;
        }
      Serial.println("Nothing Available");
    }

    if(incomingByte == 0x15){
      software_kill = 1;
      no_data = 0;
    }
    else if(incomingByte == 0x44){
      software_kill = 0;
      no_data = 0;
    } 
//    Serial.print("   Data: ");
//    Serial.println(incomingByte, BIN);
//  Serial.print("    softkill status: ");
//  Serial.1println(incomingByte);
 
  if (hardware_kill == 1){
    if (software_kill == 1){
//    Serial.println("    Normal Operations");
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);  
    }
    else{
//    Serial.println("    Kill Vehicle");
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);  
    }
  }
  else{
//    Serial.println("    Kill Vehicle");
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
  }
  delay(200);
}

