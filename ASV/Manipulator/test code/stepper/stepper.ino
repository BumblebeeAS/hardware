#include <step.h>

Step a;

/*************************************************************************************
 * 
 * 		  only moveStepper is a public function
 *      rmb to connect limit switch
 * 
 ************************************************************************************/

void setup(void) {
  Serial.begin(115200);
  a.Init();
  a.enable = 1;
  a.target = 20;
}
int left = 0;
void loop(void) {
////  
//  for(int i=0; i<=305; i++) {                 // basic code of movement,  possible reason of not moving: pin config wrong ,
//    a.step1();                                //                                                         limit switch stopping u   ,      
//    a.step2();                                //                                                         motor chip died
//    a.step3();
//    a.step4();
//  }
  a.stopMotor();
//  delay(1000);
//-------------------------------------------
//  a.moveLeft();                               // limit switch or chip died
//------------------------------------------

//  a.moveRight();
//  delay(5);
//------------------------------------------
  
//    a.moveStepper();                            //  not enabled, ,                                
//    delay(100);                               //  it thinks it is at the wrong position(out of range) 
//Serial.println(a.target);                     // chip died,   limit switch

//----------------------------------------------------------------
  if (left == 1) {
    for (int i=0; i<1000; i++) {
      if (a.checkLeftLimit()) {
        break;
      }
      a.step1();
      a.step2();
      a.step3();
      a.step4();
    }
    a.stopMotor();
    left = 0;
  } 
  
  if (left == 0) {
    for (int i=0; i<1000; i++) {
      if (a.checkRightLimit()){
        break;
      }
      a.step3();
      a.step2();
      a.step1();
      a.step4();
    }
    a.stopMotor();
    left = 1;
  }
//-------------------------------------------------------------
}




