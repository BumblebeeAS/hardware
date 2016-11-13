#include <XBee.h>
#include <SoftwareSerial.h>

// create the XBee object
XBee xbee = XBee();
uint8_t payload[] = { 0, 0 };
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40D840B7);

ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

int8_t sentByte;

SoftwareSerial mySerial(5, 4); // RX, TX

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
mySerial.begin(9600);
xbee.setSerial(mySerial);
pinMode(10, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
if (digitalRead(10)== HIGH){
  sentByte = 0x15;
}
else{
  sentByte = 0x44;
}
//  mySerial.write(sentByte);
  Serial.print("sendbyte: ");
  Serial.println(sentByte, HEX);
  payload[0] = sentByte;
  Serial.print("payload: ");
  Serial.println(payload[0], HEX);
  xbee.send(zbTx);

  // after sending a tx request, we expect a status response
   // wait up to half a second for the status response
   if (xbee.readPacket(100))
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
   delay(100);
}
