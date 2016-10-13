#include <SoftwareSerial.h>

uint8_t incomingByte;
int hardware_kill;
int no_data;
int software_kill;

SoftwareSerial mySerial(3, 2); // RX, TX

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
mySerial.begin(9600);
incomingByte = 3;
hardware_kill = 1;
software_kill = 1;
no_data = 0;
pinMode(7, INPUT);
pinMode(9, OUTPUT);
pinMode(10, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(7) == HIGH){
    hardware_kill = 1;
  }
  else if (digitalRead(7) == LOW){
    hardware_kill = 0; 
  }
  //Serial.print("hardkill status: ");
  //Serial.println(hardware_kill);

  // read the incoming byte:
  if (mySerial.available()){
    
    incomingByte = mySerial.read();
     
    if(incomingByte == 0x15){
      software_kill = 1;
    }
    else if(incomingByte == 0x44){
      software_kill = 0;
    } 
    no_data = 0;
  //  Serial.print("   Data: ");
  //  Serial.println(incomingByte, BIN);
  }
  else{
    no_data++;
//    Serial.print("     count: ");
//    Serial.print(no_data); 
    if(no_data == 50){
      software_kill = 0;
      no_data = 0;
    }   
  }
  
//  Serial.print("    softkill status: ");
//  Serial.println(incomingByte);
 
  if (hardware_kill == 1){
    if (software_kill == 1){
//    Serial.println("    Normal Operations");
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);  
    }
    else{
//    Serial.println("    Kill Vehicle");
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);  
    }
  }
  else{
//    Serial.println("    Kill Vehicle");
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
  }
  delay(100);
}

