#include <XBee.h>

// create the XBee object
XBee xbee = XBee();
uint8_t payload[] = { 0, 0 };
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B9779);

ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

int8_t sentByte = 0x15;
uint8_t inByte;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
}

void loop() {

  if (Serial.available()) {
    inByte = Serial.read();
  
    if (inByte == 0x31){              //Press 1 to return to normal operations (turn on contactors)
      Serial.println("normal");
      sentByte = 0x15;
    }
    else{
      Serial.println("KILL");
      sentByte = 0x44;
    }
  }

//  Serial.print("sendbyte: ");
//  Serial.println(sentByte, HEX);
  payload[0] = sentByte;
  Serial.print("payload: ");
  Serial.println(payload[0], HEX);
  xbee.send(zbTx);

  // after sending a tx request, we expect a status response
   // wait up to half a second for the status response
   if (xbee.readPacket(500))
   {
     // got a response!
     // should be a znet tx status              
     if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE)
     {
        xbee.getResponse().getZBTxStatusResponse(txStatus);
        // get the delivery status, the fifth byte
        /*if (txStatus.getDeliveryStatus() == SUCCESS) 
        {
          // success.  time to celebrate
          //flashLed(statusLed, 5, 10);
          Serial.println("Ack");
        } 
        else
        {
          // the remote XBee did not receive our packet. is it powered on?
          //flashLed(errorLed, 1, 50);
          Serial.println("No Acknowledgement");
        }
        */
      }      
   } 
   else 
   {
     // local XBee did not provide a timely TX Status Response -- should not happen
     //flashLed(errorLed, 5, 50);
     Serial.println("Sender Error");
   }
   delay(1000);
}
