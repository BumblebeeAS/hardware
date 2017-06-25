#include <Servo.h>

#define MAX_SIGNAL 2000
#define STOP 1500
#define MIN_SIGNAL 1000
#define MOTOR_PIN 9

int motorspeed;

Servo motor;

void setup() {
  pinMode(8, OUTPUT);
  analogWrite(8, 64);
  Serial.begin(9600);
  Serial.println("Program begin...");

  motor.attach(MOTOR_PIN);
  
  Serial.println("This program will calibrate the ESC.");

  Serial.println("Now writing neutral output.");
  Serial.println("Turn on power source, then wait 2 seconds and press any key.");
  motor.writeMicroseconds(MAX_SIGNAL);

}

void loop() {  
    // Wait for input
  while (!Serial.available());
  Serial.read();  

  // Send stop output
  Serial.println("Sending stop output");
  motor.writeMicroseconds(STOP);
  
  // Wait for input
  while (!Serial.available());
  Serial.read();
  
  // Send maximum output
  Serial.println("Sending maximum output");
  motor.writeMicroseconds(MAX_SIGNAL);
  
  // Wait for input
  while (!Serial.available());
  Serial.read();
  
  // Send min output
  Serial.println("Sending minimum output");
  motor.writeMicroseconds(MIN_SIGNAL);
  
    // Wait for input
  while (!Serial.available());
  Serial.read();  

  // Send stop output
  Serial.println("Sending stop output");
  motor.writeMicroseconds(STOP);
}
