int SWA = 22;

void setup (void) {
  Serial.begin(115200);
  pinMode(SWA,INPUT);
}

void loop(void) {
  if (digitalRead(SWA)==LOW) {
    Serial.println("LOW");
  } else {
    Serial.println("HIGH");
  }
  delay(100);
}

