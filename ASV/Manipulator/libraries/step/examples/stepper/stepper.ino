#include <step.h>

Step a;

int SWA = 22;

void setup(void) {
  Serial.begin(115200);
  a.Init();
  pinMode(SWA,INPUT);
}

void loop(void) {
    int n = 0;
//  for(int i=0; i<=305; i++) {
//    a.step1();
//    a.step2();
//    a.step3();
//    a.step4();
//  }
//  a.stopMotor();
//  delay(1000);

//  a.moveLeft(300);
//  a.moveRight(300);
//  delay(2000);

//    if(Serial.available() > 0) {
//      n = Serial.read();
//    }

//    a.moveStepper(20);
//    delay(2000);   


}


