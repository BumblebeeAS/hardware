#include <SSD1306_text.h>
SSD1306_text display(7);
int value = 0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  analogReference(EXTERNAL);
  display.init();
  display.clear();
}

void loop() {
  // put your main code here, to run repeatedly:

  value = analogRead(A7);

  display.setTextSize(1, 1);
  display.setCursor(4, 0);
  display.print(value);
  delay (500);

}
