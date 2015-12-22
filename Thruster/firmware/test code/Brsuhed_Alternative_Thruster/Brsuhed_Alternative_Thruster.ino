#include <Arduino.h>
#include <Math.h>
#include <Brushed.h>

void reset_All(void);
void run_All(void);
void stop_All(void);

//Direction Register, Direction Register, IO Register,
Brushed Thruster1(DDRA, DDRA, PORTA, CHN_1_SR, CHN_1_RESET, CHN_1_PWM, CHN_1_FF1, CHN_1_FF2, PWM1);

void setup() {
  // put your setup code here, to run once:
  init();

}

void loop() {
  // put your main code here, to run repeatedly:

}


void reset_All()
{
  Thruster1.reset();
}


void run_All()
{
  Thruster1.run(1500);
}

void stop_All()
{
  Thruster1.stop() ;
}
