// Example 14: XBee S2 RX Analog

// This program has been modified by Liaw Hwee Choo, Jan 2013.
// This program requires XBee Lib ver. 0.4 and above.

/* Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include <XBee.h>

/*
This example is for Series 2 XBee
Receives a ZB RX packet and sends the packet data to PC.
Error led is flashed if an unexpected packet is received
*/

XBee xbee               = XBee();

// create reusable response objects for responses we expect to handle 
XBeeResponse response   = XBeeResponse();
ZBRxResponse        rx  = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

void setup() 
{
  
  // start xbee in serial 2
  Serial2.begin(9600);
  xbee.setSerial(Serial2);
// xbee.begin(9600);
  Serial.begin(9600);
}

// continuously reads packets, looking for ZB Receive or Modem Status
void loop() 
{
    xbee.readPacket();
    flashLed(dataLed, 1, 10);  
    if (xbee.getResponse().isAvailable())
    {
      // got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) 
      {
        // got a zb rx packet
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(rx);
        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) 
        {
           // the sender got an ACK
           flashLed(statusLed, 5, 10); 
        } 
        else 
        {
           // we got it (obviously) but sender didn't get an ACK
           flashLed(errorLed, 2, 50);
        }
        // get data
        value = rx.getData(0) << 8 | rx.getData(1);
        voltage = value * 5.0 / 1024.0;
        Serial.print("Voltage: ");
        Serial.print(voltage, 4);
        Serial.println(" V ");
        delay(500);
        digitalWrite(led, LOW); // turn the led pin off
        delay(500); 
      } 
      else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE)
      {
        xbee.getResponse().getModemStatusResponse(msr);
        // the local XBee sends this response on certain events, 
        // like association/dissociation
        if (msr.getStatus() == ASSOCIATED) 
        {
          // yay this is great.  flash led
        } 
        else if (msr.getStatus() == DISASSOCIATED) 
        {
          // this is awful.. flash led to show our discontent
        }
        else 
        {
          // another status
        }
      }
      else
      {
      	// not something we were expecting
      }
    }
}
