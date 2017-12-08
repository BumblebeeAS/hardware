void setup() {
  // put your setup code here, to run once:
  Serial2.begin(9600);
  Serial.begin(9600);
}
byte haha = 'a';
void loop() {
  // put your main code here, to run repeatedly:
  Serial2.write(haha);
  Serial.print(haha);
  delay(500);
}
