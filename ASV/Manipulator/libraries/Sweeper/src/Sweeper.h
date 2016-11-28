#ifndef _SWEEPER_H
#define _SWEEPER_H

#include <Arduino.h>
#include <Servo.h>

class Sweeper {
private:
	Servo servo;
	int error;		            //  550 is 0 deg, 2500 is 180 deg,1525 is 90deg ---!!!!!!!!---- different for each servo
	int target;                 // wrt from x axis  1deg 10.83333333µs
	int zero_pt;				//  y=975/90*x+1525
								// ********THIS IS POSITION************
public:
	void attach(int pin);
	void detach(void);
	int currTarget(void);
	void updateTargetPos(int target);
	void updateCurrPos(int move);
};          // end of Sweeper class 

           // servo reaction speed 6V no load 0.21s/60degree      0.63s/180deg       3.5ms/deg
           // servo 0-180 degree 550-2500

#endif

