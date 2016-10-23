#ifndef _STEP_H
#define _STEP_H

#include <Arduino.h>

class Step {
  private:
  public:
    int inA1; // input 1 of the stepper
    int inA2; // input 2 of the stepper
    int inB1; // input 3 of the stepper
    int inB2; // input 4 of the stepper

    int SWA;   // limit switch A
    int SWB;   // limit switch B

    uint8_t pos;
    uint8_t step;
    uint16_t dist;
    uint8_t dir;

    int stepDelay;      // Delay between steps in milliseconds

    void step1(void);
    void step2(void);
    void step3(void);
    void step4(void);
    void stopMotor(void);
    
    void checkDir(uint8_t cm);
    void moveLeft(uint8_t cm);
    void moveRight(uint8_t cm);
    void distance(uint8_t cm);
    bool checkLimit(void);

  
    Step(void); 

    void Init(void);
    void moveStepper(uint8_t cm);


};

#endif 

