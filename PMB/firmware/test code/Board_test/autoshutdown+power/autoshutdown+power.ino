#define boardPower 3
#define vehiclePower 6
int time_delay0 = 5000;
int time_delay1 = 10000;
int start = 0;
bool started = false;

void setup() {
  Serial.begin(9600);
  pinMode(boardPower, OUTPUT);
  pinMode(vehiclePower, OUTPUT);
  start = millis();
  digitalWrite(boardPower, LOW);
  digitalWrite(vehiclePower, LOW);
}

void loop() {
  Serial.println(".");
 if ((millis() - start) > time_delay0 && !started){
   Serial.println("ONNNNNNNNNN");
   digitalWrite(vehiclePower, HIGH);
   started = true;
 }
 if ((millis() - start) > time_delay1){
   Serial.println("SHUTDOWNNNNN");
   digitalWrite(boardPower, HIGH);
   digitalWrite(vehiclePower, LOW);
 }
}
