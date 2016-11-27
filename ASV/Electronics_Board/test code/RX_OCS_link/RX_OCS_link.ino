#include <Xbee.h>

XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
XBee xbee = XBee();
//uint8_t payload[5] = { 0, 0, 0, 0, 0 };
uint8_t *payload;
uint8_t cmd[] = { 'D','B' };
AtCommandRequest atRequest = AtCommandRequest();
AtCommandResponse atResponse = AtCommandResponse();

void setup()
{

  // create the XBee object
  Serial.begin(115200);
  
  xbee.setSerial(Serial);

  XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40D840B7);
}

void loop()
{
  /* add main program code here */
	xbee.readPacket();
	if (xbee.getResponse().isAvailable())
	{
		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE)
		{
			xbee.getResponse().getZBRxResponse(rx);
			payload = rx.getData();
			Serial.print("data:");
			for (int i = 0; i < rx.getDataLength();i++)
			{
				Serial.write(payload[i]);
				Serial.print(" ");
			}
			// set command to DB
			atRequest.setCommand(cmd);
			sendAtCommand();	
		}
		
	}

}

void sendAtCommand() {
	Serial.println("Sending command to the XBee");

	// send the command
	xbee.send(atRequest);

	// wait up to 5 seconds for the status response
	if (xbee.readPacket(5000)) {
		// got a response!

		// should be an AT command response
		if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
			xbee.getResponse().getAtCommandResponse(atResponse);

			if (atResponse.isOk()) {
				Serial.print("Command [");
				Serial.print(atResponse.getCommand()[0]);
				Serial.print(atResponse.getCommand()[1]);
				Serial.println("] was successful!");

				if (atResponse.getValueLength() > 0) {
					Serial.print("Command value length is ");
					Serial.println(atResponse.getValueLength(), DEC);

					Serial.print("Command value: ");

					for (int i = 0; i < atResponse.getValueLength(); i++) {
						Serial.print(atResponse.getValue()[i], HEX);
						Serial.print(" ");
					}

					Serial.println("");
				}
			}
			else {
				Serial.print("Command return error code: ");
				Serial.println(atResponse.getStatus(), HEX);
			}
		}
		else {
			Serial.print("Expected AT response but got ");
			Serial.print(xbee.getResponse().getApiId(), HEX);
		}
	}
	else {
		// at command failed
		if (xbee.getResponse().isError()) {
			Serial.print("Error reading packet.  Error code: ");
			Serial.println(xbee.getResponse().getErrorCode());
		}
		else {
			Serial.print("No response from radio");
		}
	}
}
