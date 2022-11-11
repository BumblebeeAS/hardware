//###################################################
//
//___.  ___.                         
//\_ |__\_ |__ _____    _________  __
// | __ \| __ \\__  \  /  ___/\  \/ /
// | \_\ \ \_\ \/ __ \_\___ \  \   / 
// |___  /___  (____  /____  >  \_/  
//     \/    \/     \/     \/        
//
// Ballshooter for BBASV 3.0
// Code for ASV 3-0 Ballshooter Board: 
// https://365.altium.com/files/7BB87AB3-1B27-495B-8D2B-19159F3FFB3A?openedFrom=files&variant=%5BNo%20Variations%5D
//
// Board functions: 
// 1. Run firing and cocking mechanism (2 steppers on stepper slot 2 and 3, 1 servo on servo_3 slot)
// 2. Run reloading mechanism (1 stepper) 
// 3. Run aiming mechanism
// 
// Todo: 
// 1. Add CAN 
// 2. Add reloading mechanism
//
// Written by Titus Ng 
// Change log v0.1: 
// Inital commit with firing mechanism
//###################################################

#include "define.h" 
#define DEBUG
void setup() {
  initialize_cocking();             // Cocking steppers
  initialize_latch_release();       // latch release servo
  initialize_serial();     
          
  // CAN init 
  CAN_init();
  CAN_mask();
}

void initialize_serial(void) {
  Serial.begin(115200);
  Serial.println("Hi I'm ASV 3 Ballshooter!");
  Serial.println("Serial controls:");
  Serial.println("1: Cocking motors forward");
  Serial.println("2: Cocking motors backward");
  Serial.println("3: Fire (release latch)");
}
//===========================================
//
//    Firing & Cocking initialization
//
//===========================================
void initialize_cocking(void) {
  // declare as output
  pinMode(dir3cock, OUTPUT);
  pinMode(step3cock, OUTPUT);
  pinMode(MS13, OUTPUT);
  pinMode(MS23, OUTPUT);
  pinMode(dir2cock, OUTPUT);
  pinMode(step2cock, OUTPUT);
  pinMode(MS12, OUTPUT);
  pinMode(MS22, OUTPUT);

  // set to 1/16 microstep
  digitalWrite(MS12, HIGH);
  digitalWrite(MS22, HIGH);  
  digitalWrite(MS13, HIGH);
  digitalWrite(MS23, HIGH);
}

//===========================================
//
//    Firing & Cocking movement
//
//===========================================
void initialize_latch_release(void) {
  servo_latch.attach(LATCH_PIN);
  servo_latch.write(START_ANGLE);
}

void movecock(int dir) {
  digitalWrite(dir2cock, dir);
  digitalWrite(dir3cock, dir);
  for (int i = 0; i < stepsPerRevolution * microstep * 1; i++) {
    digitalWrite(step2cock, HIGH);
    digitalWrite(step3cock, HIGH);
    delayMicroseconds(stepperdelay);
    digitalWrite(step2cock, LOW);     
    digitalWrite(step3cock, LOW);
    delayMicroseconds(stepperdelay);
  }
  dir ? Serial.println("Moved cocking motors forward") : Serial.println("Moved cocking motors backward"); 
}

void release_latch(void) {
    servo_latch.write(ENGAGE_ANGLE);
    delay(RELEASE_DELAY);
    servo_latch.write(START_ANGLE);
    Serial.println("3: Released latch");
}

//===========================================
//
//    CAN functions
//
//===========================================
// Initialize CAN bus 
void CAN_init() {
  if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 1000k
    Serial.println("CAN init ok!");
  }
  else {
    Serial.println("CAN init fail");
    Serial.println("Init CAN again");
    delay(100);
    CAN_init();
  }
}


// Initialize CAN mask using truth table 
/*
  Truth table
  mask  filter    id bit  reject
  0     X         X       no
  1     0         0       no
  1     0         1       yes
  1     1         0       yes
  1     1         1       no
  Mask 0 connects to filt 0,1
  Mask 1 connects to filt 2,3,4,5
  Mask decide which bit to check
  Filt decide which bit to accept
*/
void CAN_mask() {
}


void loop() {
  #ifdef DEBUG
  int val = 0;
  val = Serial.parseInt();
  if (val != 0) {
    Serial.print("I got: ");
    Serial.println(val, DEC);
    if (val == 1) { 
      movecock(1);
    } else if (val == 2) {
      movecock(0);
    } else if (val == 3) {
      release_latch();
    }
  }
  delay(100);
  #endif
}
