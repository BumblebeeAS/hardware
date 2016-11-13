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
  a.target = 30;
}

void loop(void) {
//  
//  for(int i=0; i<=305; i++) {                 // basic code of movement,  possible reason of not moving: pin config wrong ,
//    a.step1();                                //                                                         limit switch stopping u   ,      
//    a.step2();                                //                                                         motor chip died
//    a.step3();
//    a.step4();
//  }
//  a.stopMotor();
//  delay(1000);
//-------------------------------------------
//  a.moveLeft();                               // limit switch or chip died
//------------------------------------------
//  a.moveRight();
//  delay(5);
//------------------------------------------
//Serial.println(a.checkLimit());               //  1 check is it due to the limit switch causing stepper not to move    
    a.moveStepper();                            //  not enabled, ,                                
//    delay(100);                               //  it thinks it is at the wrong position(out of range) 
//Serial.println(a.target);                     // chip died,   limit switch
}


