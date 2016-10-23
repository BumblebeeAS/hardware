#include <Arduino.h>
#include <step.h>

#define LEFT 0
#define RIGHT 1
#define Ratio 11.595744680

Step::Step(void) {
	inA1 = 2; // input 1 of the stepper
    inA2 = 4; // input 2 of the stepper
    inB1 = 3; // input 3 of the stepper
    inB2 = 5; // input 4 of the stepper

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

	pos = 47;		//	centimeter
    stepDelay = 1;	//	ms
    dist = 0;		//	cm


    // for (int i=0;i<545;i++) {
    // 	if(checkLimit()) {
    // 		break;
    // 	}
    // 	step1();
    // 	step2();
    // 	step3();
    // 	step4();
    // }

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

void Step::moveLeft(uint8_t dist) {
	int i = 0;
	for (i=0; i<=dist; i++) {
		if (checkLimit() || pos == 0) {
			break;
		}
		step1();
		step2();
		step3();
		step4();
	}
	stopMotor();	
}

void Step::moveRight(uint8_t dist) {
	int i=0;
	for (i=0; i<=dist; i++) {
		if (checkLimit() || pos == 47) {
			break;
		}
		step3();
		step2();
		step1();
		step4();			
	}
	stopMotor();
}

void Step::checkDir(uint8_t cm) {
	dir = (cm-pos>=0) ? RIGHT : LEFT;		
	// distance measured from left,  leftmost = 0
}

void Step::distance(uint8_t cm) {
	dist = 0;                  	   			// check distance to be moved
	if (cm>=pos) {  
	dist = (round)(Ratio*(cm-pos));
	} else {
	dist = (round)(Ratio*(pos-cm));
	}
}

void Step::moveStepper(uint8_t cm) {       // left most is 0   viewed from the back
	distance(cm);
	checkDir(cm);
	if (dir == LEFT) {
		moveLeft(dist);
		pos -=dist;
	} else if (dir == RIGHT) {
		moveRight(dist);
		pos+=dist;
	}	
	stopMotor();
	dist = 0;
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


		//   1234		from right to left   viewed from back

		//	 3214		from left to right   viewed from bacl