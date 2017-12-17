void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600); //RS232 on OCS
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial1.write("Hello World");
  delay(10);
  while (Serial1.available()){
    byte input = Serial1.read();
    Serial.print(input);
  }
  delay(1000);
}
