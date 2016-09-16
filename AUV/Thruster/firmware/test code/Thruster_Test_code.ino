#include <Brushed.h>
#include <Servo.h>
#include <Thrusters.h>

Brushed motor;
Thrusters videoray(45, 46);

void setup()
{
  motor.begin();
}

void loop()
{
  videoray.mov(2500);
  motor.runAll(2500);
  delay(5000);
  videoray.mov(0);
  motor.stopAll();
  delay(5000);
}
