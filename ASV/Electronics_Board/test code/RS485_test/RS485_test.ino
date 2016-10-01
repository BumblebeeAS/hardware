/*
* Use a RS-485-to-USB cable to connect the RS485 side of the transceiver
* Open a serial monitor for that COM port,
* and you should be able to transmit and receive through that port
*/

#define RE1 22
#define DE1 24
#define RE2 26
#define DE2 28

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial1.begin(19200);
  Serial2.begin(19200);
  pinMode(RE1, OUTPUT);
  pinMode(DE1, OUTPUT);
  pinMode(RE2, OUTPUT);
  pinMode(DE2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int i;
  char c;
  string received1 = "";
  string received2 = "";
  
  //Driver mode
  digitalWrite(RE1, HIGH);
  digitalWrite(RE2, HIGH);
  digitalWrite(DE1, HIGH);
  digitalWrite(DE2, HIGH);
  
  Serial1.print("T: ");
  Serial2.print("T: ");
  for (i = 0; i < 10; i++)
  {
  Serial1.print(i);
  Serial2.print(i);

  }
    
  for (c = 'A'; c - 'A' < 26; c++)
  {
  Serial1.print(c);
  Serial2.print(c);

  }

  delay(25);
    
  //Receiver mode

  digitalWrite(RE1, LOW);//Must disable driver before enable receiver
  digitalWrite(RE2, LOW);
  digitalWrite(DE1, LOW);
  digitalWrite(DE2, LOW);
  
  while(!Serial1.available() && !Serial2.available());
  while(Serial1.available())
    received1 = Serial1.read();
  while(Serial2.available())
    received2 = Serial2.read();
  
  //Driver mode
  //delay(25);
  digitalWrite(RE1, HIGH);
  digitalWrite(RE2, HIGH);
  digitalWrite(DE1, HIGH);
  digitalWrite(DE2, HIGH);
  
  Serial1.print("\nR: ");
  Serial1.write(received1);
  Serial1.print("\n");
  Serial2.print("\nR: ");
  Serial2.write(received2);
  Serial2.print("\n");
}
