#ifndef Thrusters_h
#define Thrusters_h

#include "Arduino.h"
#include <Servo.h>

class Thrusters
{
 public:
 Thrusters(int pin1, int pin2);
 void mov(int input1,int input2);
 void movThruster1(int input1);
 void movThruster2(int input2);


 private: 
 void acc(int input1, int input2 ,int input3, int input4);
 int thrusterForward1(int input);
 int thrusterReverse1(int input);
 int thrusterForward2(int input);
 int thrusterReverse2(int input);
 int thrusterStop(void);
 Servo esc1, esc2;
 int motor1, motor2;
 int increment_1(int, int);
 int increment_2(int, int);
};

#endif