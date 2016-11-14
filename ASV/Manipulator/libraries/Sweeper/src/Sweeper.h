#ifndef _SWEEPER_H
#define _SWEEPER_H

#include <Arduino.h>
#include <Servo.h>

class Sweeper {
private:
	Servo servo;
	int updateInterval;             // interval between updates
	unsigned long lastUpdate;       // last update position
	int error;		                //  550 is 0 deg, 2500 is 180 deg,1525 is 90deg ---!!!!!!!!---- different for each servo
	                    		    // wrt from x axis  1deg 10.83333333µs
									//  y=975/90*x+1525
									// ********THIS IS POSITION************
public:

	uint8_t enable;
	uint8_t target;

	Sweeper(int interval);
	void attach(int pin);
	void detach(void);
	void update(int move);
};          // end of Sweeper class 

           // servo reaction speed 6V no load 0.21s/60degree      0.63s/180deg       3.5ms/deg
           // servo 0-180 degree 550-2500

#endif

