//SND: 0x0013A200, 0x416B976D
//RCV: 0x0013A200, 0x416B9779

#include <XBee.h>

XBee xbee = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response   = XBeeResponse();
ZBRxResponse rx  = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

uint8_t payload[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int no_data = 0;
int software_kill = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(115200);
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
      //get data
      
      Serial.print("Incoming Data, length = ");
      uint8_t len = rx.getDataLength();
      Serial.print(len, DEC);
      Serial.print(", |");
      for (uint8_t i=0; i< len; i++)
        Serial.print((char)rx.getData(i));
      Serial.println("|");
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
    Serial.println("Nothing Available");
  }
  delay(1000);
}
