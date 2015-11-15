#include <SPI.h>
#include <can.h>
#include <mcp_can.h>

MCP_CAN CAN(10);                                      // Set CS to pin 10

void setup()
{
	  Serial.begin(115200);

START_INIT:

	  if(CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
        Serial.println("CAN Sender...");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000101);
	SPI.transfer(0x0F);
	SPI.transfer(0xE0);
	SPI.transfer(MODE_NORMAL);
	digitalWrite(10,HIGH);
	
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000101);
	SPI.transfer(0x0F);
	SPI.transfer(0b00000100);
	SPI.transfer(CLKOUT_ENABLE);
	digitalWrite(10,HIGH);
	
	/*
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000101);
	SPI.transfer(0x0F);
	SPI.transfer(0b00001000);
	SPI.transfer(MODE_ONESHOT); //One shot mode
	digitalWrite(10,HIGH);
	*/
	byte test = 0;

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000011);
	SPI.transfer(0x0F);
	test = SPI.transfer(0x00);
	Serial.print("mode:");
	Serial.println(test);
	digitalWrite(10,HIGH);

	//Set transmit interrupt
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x2B);
	test = SPI.transfer(0x04);
	digitalWrite(10,HIGH);

	//Transmission
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x31);
	SPI.transfer(0b00000000); //Higher address
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x32); //Lower address
	SPI.transfer(0b00000000);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x35); //set data length
	SPI.transfer(0b00001000);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x36); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x37); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x38); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x39); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x3A); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x3B); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x3C); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x3D); //Set data byte
	SPI.transfer(2);
	delay(10);
	digitalWrite(10,HIGH);

	//Transmit set request
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000010);
	SPI.transfer(0x30);
	SPI.transfer(0b00001010); //Transmit register setting
	delay(10);
	digitalWrite(10,HIGH);


}

void loop()
{
	
	byte testing = 0;
	/*
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000011);
	SPI.transfer(0x30);
	testing = SPI.transfer(0x00);
	Serial.print("c:");
	Serial.print(testing);
	digitalWrite(10,HIGH);
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000011);
	SPI.transfer(0x1C);
	testing = SPI.transfer(0x00);
	Serial.print(" tec:");
	Serial.print(testing);
	digitalWrite(10,HIGH);
	*/
	/*
	digitalWrite(10,LOW);	
	SPI.transfer(0b00000011);
	SPI.transfer(0x1D);
	testing = SPI.transfer(0x00);1
	*/
	//Serial.print(" rec:");
	//Serial.println(testing);
	digitalWrite(10,HIGH);

	
	//Transmit set request
	digitalWrite(10,LOW);	
	SPI.transfer(0b10000001); //RTS
	digitalWrite(10,HIGH);
}

