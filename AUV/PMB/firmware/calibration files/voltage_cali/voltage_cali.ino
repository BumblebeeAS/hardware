
int value = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  analogReference(EXTERNAL);

}

void loop() {
  // put your main code here, to run repeatedly:

  value = analogRead(A7);

  Serial.println(value);
  
  delay (500);

}
