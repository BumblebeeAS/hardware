uint8_t inByte;

void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW);
}

void loop() {
  // read from port 1, send to port 0:
  if (Serial.available()) {
      
    inByte = Serial.read();
    
    if (inByte == 0x31){              //Press 1 to return to normal operations (turn on contactors)
      digitalWrite(11, HIGH);
      Serial.println("normal");
    }
    else {                            //Press any other key to kill contactors
      digitalWrite(11, LOW);
      Serial.println("KILL!");
    }
  } 
}
