#include"SmartMotor.h"

SmartMotor lars;

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  lars.init();
  /*char str[] = "HI";
  if(str[2] == '\0')
  Serial.print(str);*/
  lars.startup();
  
  lars.setAcceleration(4000);
  lars.setVelocity(50000);
  lars.setPosition(50000);

  //lars.requestAcceleration();
  lars.setRun(true);
  delay(5000);
  lars.setRun(false);
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
lars.readMessage();
}
