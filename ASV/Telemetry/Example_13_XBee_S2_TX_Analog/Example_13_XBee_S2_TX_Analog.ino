#include <XBee.h>

// create the XBee object
XBee xbee = XBee();

uint8_t payload[] = { 0, 0 };

// SH + SL Address of receiving XBee                             // Remote XBee    
// XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x407C48E0); // cor_1
// XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x407C48E2); // end_1
// XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x407C609E); // cor_2, XB24-B, 1147
   XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40B18812); // end_2, XB24-B, 1347
   
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

void setup()
{

  // start xbee in serial 2
  Serial2.begin(9600);
  xbee.setSerial(Serial2);
  Serial.begin(9600);
// xbee.begin(9600);
  pinMode(A8,OUTPUT);
}

int h = 0x68;
int i = 0x69;

void loop()
{   
   // convert 10-bit reading into two bytes and place in payload
   payload[0] = h;
   payload[1] = i;
    
   xbee.send(zbTx);
   Serial.println("sending");
   // flash TX indicator
    
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
        if (txStatus.getDeliveryStatus() == SUCCESS) 
        {
          // success.  time to celebrate
          digitalWrite(A8,HIGH);
          delay(100);
          digitalWrite(A8,LOW);
        } 
        else
        {
          // the remote XBee did not receive our packet. is it powered on?
        }
      }      
   } 
   else 
   {
     // local XBee did not provide a timely TX Status Response -- should not happen
   }
   delay(1000);
}