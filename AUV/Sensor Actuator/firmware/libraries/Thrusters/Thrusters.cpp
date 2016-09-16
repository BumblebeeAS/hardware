#include <Arduino.h>
#include <Servo.h>
#include <Thrusters.h>
#include <Math.h>

#define RATE 0.05  /// MUST BE LESS THAN 1
String sinit = "";

Thrusters::Thrusters(int pin1, int pin2)
{
	motor1 = pin2;
	motor2 = pin1;
}

int Thrusters::increment_1(int throttleold, int throttlenew){
	int i = throttleold;
	if (throttlenew > throttleold){
    i = i + ceil(RATE * (throttlenew - throttleold));
		esc1.writeMicroseconds(i);
	}else if (throttleold > throttlenew){
    i = i - ceil(RATE * (throttleold - throttlenew));
		esc1.writeMicroseconds(i);
	}
	Serial.println(sinit + "ESC1 Throttle: " + i);
	return i;
}

int Thrusters::increment_2(int throttleold, int throttlenew){
	int i = throttleold;
	if (throttlenew > throttleold){
    i = i + ceil(RATE * (throttlenew - throttleold));
		esc2.writeMicroseconds(i);
	}else if (throttleold > throttlenew){
    i = i - ceil(RATE * (throttleold - throttlenew));
		esc2.writeMicroseconds(i);
	}
	Serial.println(sinit + "ESC2 Throttle: " + i);
	return i;
}

void Thrusters::acc(int throttleOld1, int throttle1, int throttleOld2, int throttle2)
{

	if (throttleOld1 != throttle1 && throttleOld2 != throttle2){
		acc(increment_1(throttleOld1, throttle1), throttle1, increment_2(throttleOld2, throttle2), throttle2);
    return;
	}
  else if(throttleOld1 != throttle1 && throttleOld2 == throttle2){
    acc(increment_1(throttleOld1, throttle1), throttle1, increment_2(throttleOld2, throttle2), throttle2);
    return;
  }
  else if(throttleOld1 == throttle1 && throttleOld2 != throttle2){
    acc(increment_1(throttleOld1, throttle1), throttle1, increment_2(throttleOld2, throttle2), throttle2);
    return;
  }

}

void Thrusters::mov(int input1, int input2)
{
esc1.attach(motor1);
esc2.attach(motor2);
int throttle1, throttleOld1;
int throttle2, throttleOld2;

throttleOld1 = esc1.readMicroseconds();
throttleOld2 = esc2.readMicroseconds();

if(input1 > 0)    {throttle1 = thrusterForward1(input1);}
if(input1 < 0)    {throttle1 = thrusterReverse1(input1);}
if(input1 == 0)   {throttle1 = thrusterStop();}

if(input2 > 0)    {throttle2 = thrusterForward2(input2);}
if(input2 < 0)    {throttle2 = thrusterReverse2(input2);}
if(input2 == 0)   {throttle2 = thrusterStop();}


acc(throttleOld1, throttle1,throttleOld2, throttle2);
return;
}

void Thrusters::movThruster1(int input1)
{
esc1.attach(motor1);
esc2.attach(motor2);
int throttle1, throttleOld1;
int throttleOld2;

throttleOld1 = esc1.readMicroseconds();
throttleOld2 = esc2.readMicroseconds();

if(input1 > 0)    {throttle1 = thrusterForward1(input1);} 
if(input1 < 0)    {throttle1 = thrusterReverse1(input1);}
if(input1 == 0)   {throttle1 = thrusterStop();}

acc(throttleOld1, throttle1,throttleOld2, throttleOld2);
return;
}

void Thrusters::movThruster2(int input2)
{
esc1.attach(motor1);
esc2.attach(motor2);
int throttleOld1;
int throttle2, throttleOld2;

throttleOld1 = esc1.readMicroseconds();
throttleOld2 = esc2.readMicroseconds();

if(input2 > 0)    {throttle2 = thrusterForward2(input2);}
if(input2 < 0)    {throttle2 = thrusterReverse2(input2);}
if(input2 == 0)   {throttle2 = thrusterStop();}

acc(throttleOld1,throttleOld1,throttleOld2, throttle2);
return;
}



int Thrusters::thrusterForward1(int input)
{
int throttle = map(input, 1, 3200, 1606, 1942);
//int throttle = map(input, 1, 3200, 1610, 2010);
//int throttle = map(input, 1, 3200, 1420, 1000);
return throttle;
}

int Thrusters::thrusterReverse1(int input)
{
int throttle = map(input, -3200, -1, 1063, 1399);
//int throttle = map(input, -3200, -1, 995, 1395);
//int throttle = map(input, -3200, -1, 2000, 1580);
return throttle;
}

int Thrusters::thrusterForward2(int input)
{
int throttle = map(input, 1, 3200, 1615, 1951);
//int throttle = map(input, 1, 3200, 1592, 1992);
//int throttle = map(input, 1, 3200, 1420, 1000);
return throttle;
}

int Thrusters::thrusterReverse2(int input)
{
int throttle = map(input, -3200, -1, 1055, 1391);
//int throttle = map(input, -3200, -1, 2000, 1580);
return throttle;
}

int Thrusters::thrusterStop(void)
{
return 1500;
}