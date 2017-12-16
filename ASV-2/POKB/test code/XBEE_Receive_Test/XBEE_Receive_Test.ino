#include <SoftwareSerial.h>
#include <XBee.h>

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response   = XBeeResponse();
ZBRxResponse rx  = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

uint8_t incomingByte = 0;
int no_data = 0;
int software_kill = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
}

void loop() {
  xbee.readPacket();

  if (xbee.getResponse().isAvailable()){
    Serial.println("Available");
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) 
    {
      // got a zb rx packet
      // now fill our zb rx class
      xbee.getResponse().getZBRxResponse(rx);
      //Serial.println("Response");
      if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) 
      {
         // the sender got an ACK
         //flashLed(statusLed, 5, 10);
         //Serial.println("Ack"); 
      } 
      else 
      {
         // we got it (obviously) but sender didn't get an ACK
         //flashLed(errorLed, 2, 50);
         //Serial.println("Send nvr get Ack");
      }
      // get data
      incomingByte = rx.getData(0);
      Serial.print("Incoming Byte: ");
      Serial.println(incomingByte, HEX);

      if(incomingByte == 0x15){
        software_kill = 0;
        no_data = 0;
        Serial.println("normal");
      }
  
      else if(incomingByte == 0x44){
        software_kill = 1;
        no_data = 0;
      }
    }
  }
   
  else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE){
    Serial.println("here?");
    xbee.getResponse().getModemStatusResponse(msr);
    // the local XBee sends this response on certain events, 
    // like association/dissociation
    
    if (msr.getStatus() == ASSOCIATED){
      // yay this is great.  flash led
      //flashLed(statusLed, 5, 100);
    } 
    else if (msr.getStatus() == DISASSOCIATED){
      // this is awful.. flash led to show our discontent
      //flashLed(errorLed, 10, 50);
    }
    else{
      // another status
      //flashLed(statusLed, 10, 100);
    }
  }
  
  else {
    no_data++;
    Serial.print("Count: ");
    Serial.println(no_data);
    
    if(no_data == 20){
      software_kill = 1;
      no_data = 0;
      Serial.println("Connection Time-out KILL");
    }
    Serial.println("Nothing Available");
  }
  delay(1000);
}
