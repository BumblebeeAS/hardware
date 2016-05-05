void setup() {
  // put your setup code here, to run once:
//  analogReference(EXTERNAL);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("1: ");
  Serial.print(analogRead(0));
  Serial.print(" 2: ");
  Serial.print(analogRead(1));
  Serial.print(" 3: ");
  Serial.print(analogRead(2));
  Serial.print(" 4: ");
  Serial.print(analogRead(3));
  Serial.print(" 5: ");
  Serial.print(analogRead(6));
  Serial.print(" 6: ");
  Serial.println(analogRead(7));
  delay(1000);
}
