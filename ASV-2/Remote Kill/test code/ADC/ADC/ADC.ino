#define BATTERY_READ A2
#define LED_LOW_BATT A1

void setup() {
  // put your setup code here, to run once:
  pinMode(BATTERY_READ, INPUT);
  pinMode(LED_LOW_BATT, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  double batteryBit = analogRead(BATTERY_READ);
  double batteryVoltage = (batteryBit - 13.3) / 227.5;
  Serial.print(batteryVoltage);
  Serial.print("\n");
}
