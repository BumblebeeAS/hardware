#include <Servo.h>
#include "Thrusters.h"

#define MAX_SIGNAL 2000
#define MIN_SIGNAL 1000
#define STOP 1500
#define MOTOR_PIN1 10
#define MOTOR_PIN2 3

int motorspeed;
char inputstr[10] = { '\n' };
int serialidx = 0;

Servo motor1;
Servo motor2;
Thrusters thruster1(2, motor2, MOTOR_PIN2, 2000, 1500, 1500, 1000);

void setup() {
  Serial.begin(9600);
  Serial.println("Program begin...");
  char input;

  motor1.attach(MOTOR_PIN1);

  // Send min output
  motor1.writeMicroseconds(STOP);
  motorspeed = STOP;

  thruster1.init();
}


void loop() {
  byte input;
  // put your main code here, to run repeatedly:
  while (!Serial.available())
      thruster1.mov(motorspeed);
  input = Serial.read();
  inputstr[serialidx] = input;

  /*
      for (int i = 0; i <= serialidx; i++)
      {
        Serial.print(inputstr[i]);
      }
      Serial.println();
  */
  if (input == '\n' || input == '\r')
  {
    inputstr[serialidx] = '\0';
    if (inputstr[0] == 'u')
    {
      Serial.println("+++");
      motorspeed = motorspeed + 50;
      motor1.writeMicroseconds(motorspeed);
      Serial.println(motorspeed);
    }
    else if (inputstr[0] == 'd')
    {
      Serial.println("---");
      motorspeed = motorspeed - 50;
      motor1.writeMicroseconds(motorspeed);
      Serial.println(motorspeed);
    }
    else if (isalpha(inputstr[0]))
    {
      motor1.writeMicroseconds(STOP);
      motorspeed = STOP;
      Serial.println("STOP");
    }
    else
    {
      motorspeed = atoi(inputstr);
      Serial.println(motorspeed);
      motor1.writeMicroseconds(motorspeed);
    }
    serialidx = -1;
    Serial.println(millis());
  }
  serialidx++;
}
