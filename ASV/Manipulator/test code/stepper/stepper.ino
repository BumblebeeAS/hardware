#include <step.h>

Step a;

/*************************************************************************************
 * 
 * 		  only moveStepper is a public function
 *      rmb to connect limit switch
 * 
 ************************************************************************************/
int SWA = 22;

void setup(void) {
  Serial.begin(115200);
  a.Init();
//  pinMode(SWA,INPUT);
  a.enable = 1;
  a.target = 9;
}

void loop(void) {
//  
//  for(int i=0; i<=305; i++) {
//    a.step1();
//    a.step2();
//    a.step3();
//    a.step4();
//  }
//  a.stopMotor();
//  delay(1000);
//-------------------------------------------
//  a.moveLeft();
//------------------------------------------
//  a.moveRight();
//  delay(5);
//------------------------------------------
//Serial.println(a.checkLimit());
    a.moveStepper();
//    delay(100);
//Serial.println(a.target);  
}


