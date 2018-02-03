#include <XBee.h>
#include <SoftwareSerial.h>
#include "defines.h"

// create the XBee object
XBee xbee = XBee();
uint8_t payload[] = { 0, 0 };
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x416B9775);

ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

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
  Serial.begin(115200);
  mySerial.begin(115200);
  xbee.setSerial(mySerial);

  pinMode(3, INPUT);
  xbee_loop = millis();

  pinMode(BATTERY_READ, INPUT);
  pinMode(LED_LOW_BATT, OUTPUT);
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
     xbee_loop = millis();
  }

  batteryCheck();
}
