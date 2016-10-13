
#include <SoftwareSerial.h>
int8_t sentByte;

SoftwareSerial mySerial(5, 4); // RX, TX

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
mySerial.begin(9600);
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
  mySerial.write(sentByte);
//  Serial.print("Sending Kill Status: ");
//  Serial.println(sentByte, BIN);

  delay(300);
}
