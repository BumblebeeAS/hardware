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
  delay(100);  
  digitalWrite(RED, LOW);
  delay(1000);  
  
  digitalWrite(YELLOW, HIGH);
  delay(100);  
  digitalWrite(YELLOW, LOW);
  delay(1000);  
  
  digitalWrite(GREEN, HIGH);
  delay(100);  
  digitalWrite(GREEN, LOW);
  delay(1000);  
}
