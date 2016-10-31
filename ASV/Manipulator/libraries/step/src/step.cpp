#include <Arduino.h>
#include <step.h>

#define LEFT 0
#define RIGHT 1
// #define step_ratio 11.595744680			// 545 step loop mapped over 47cm
#define step_length 0.0706422018//0.0697247706//0.0862385321		// 1 step loop distance in cm
//					38.5			38			47
#define Stepper_looptime 6


Step::Step(void) {
inA1 = 23; // input 1 of the stepper  
inA2 = 25; // input 2 of the stepper
inB1 = 24; // input 3 of the stepper
inB2 = 26; // input 4 of the stepper
    // for testing 
    // inA1 = 2;
    // inA2 = 4;
    // inB1 = 3;
    // inB2 = 5;

/*
    correct mapping on manipulator board 
    23			PC0			STPA1		
    24			PC1			STPA2
    25			PC2			STPA3
    26			PC3			STPA4
*/

    SWA = 22;  // limit switch A
	SWB = 19;  // limit switch B
}

void Step::Init() {
	pinMode(inA1, OUTPUT);                        
	pinMode(inA2, OUTPUT);     
	pinMode(inB1, OUTPUT);     
	pinMode(inB2, OUTPUT);   

	pinMode(SWA, INPUT);  
	pinMode(SWB, INPUT);

	pos = 0.0;		//	centimeter
    stepDelay = 1;	//	ms
    dist = 0;		//	cm
    stepperTime = 0;
    target = 0;


    for (int i=0;i<545;i++) {		// move the stepper to the left most when start up
    	if(checkLimit()) {
    		break;
    	}
		step3();
		step2();
		step1();
		step4();
    }

}

void Step::step1() {
  digitalWrite(inA1, LOW);   
  digitalWrite(inA2, HIGH);   
  digitalWrite(inB1, HIGH);   
  digitalWrite(inB2, LOW);   
  delay(stepDelay);
}
void Step::step2() {
  digitalWrite(inA1, LOW);   
  digitalWrite(inA2, LOW);   
  digitalWrite(inB1, HIGH);   
  digitalWrite(inB2, HIGH);   
  delay(stepDelay);
}
void Step::step3() {
  digitalWrite(inA1, HIGH);   
  digitalWrite(inA2, LOW);   
  digitalWrite(inB1, LOW);   
  digitalWrite(inB2, HIGH);   
  delay(stepDelay);
}
void Step::step4() {
  digitalWrite(inA1, HIGH);   
  digitalWrite(inA2, HIGH);   
  digitalWrite(inB1, LOW);   
  digitalWrite(inB2, LOW);   
  delay(stepDelay);
}
void Step::stopMotor() {
  digitalWrite(inA1, LOW);   
  digitalWrite(inA2, LOW);   
  digitalWrite(inB1, LOW);   
  digitalWrite(inB2, LOW);   
}

void Step::moveRight() {
	// int i = 0;
	// for (i=0; i<=dist; i++) {
		if (checkLimit()) {
			return;
		}
		step1();
		step2();
		step3();
		step4();
	// }
	stopMotor();	
}

void Step::moveLeft() {
	// int i=0;
	// for (i=0; i<=dist; i++) {
		if (checkLimit()) {
			return;
		}
		step3();
		step2();
		step1();
		step4();			
	// }
	stopMotor();
}

void Step::checkDir() {
	dir = ((target - pos)>=0) ? RIGHT : LEFT;		
	// distance measured from left,  leftmost = 0
}

// void Step::distance()) {         	// check distance to be moved
// 	if (cm>=pos) {  
// 	dist = (round)(step_ratio*(cm-pos));
// 	} else {
// 	dist = (round)(step_ratio*(pos-cm));
// 	}
// }

void Step::moveStepper() {       // left most is 0   viewed from the back
	if (enable != 1) {
		return;
	}
	if (millis() - stepperTime >= Stepper_looptime) {
		if ((round)(pos) == target) {
			return;
		}
		// distance(cm);
		checkDir();
		if (dir == LEFT) {
			if (pos < 0.0) {
				return;
			}
			moveLeft();
			pos-=step_length;
		} else if (dir == RIGHT) {
			if (pos > 38.0) {
				return;
			}
			moveRight();
			pos+=step_length;
		}	
		stopMotor();
	}
}

bool Step::checkLimit(void) {
	// if ( digitalRead(SWB) == LOW) {
	// 	return true;
	// }
	if ( digitalRead(SWA) == LOW) {
		return true;
	}
	return false;
}
		//   1234		from left to right   viewed from back

		//	 3214		from right to left   viewed from bacl