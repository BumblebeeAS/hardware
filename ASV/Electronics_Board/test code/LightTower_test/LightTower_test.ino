#define RED 12
#define YELLOW 11
#define GREEN 13

void setup() {
  // put your setup code here, to run once:
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(RED, HIGH);
  delay(1000);  
  digitalWrite(RED, LOW);
  
  digitalWrite(YELLOW, HIGH);
  delay(1000);  
  digitalWrite(YELLOW, LOW);
  
  digitalWrite(GREEN, HIGH);
  delay(1000);  
  digitalWrite(GREEN, LOW);
}
