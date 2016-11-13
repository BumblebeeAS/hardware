#include <Servo.h>
#include <IMU.h>
#include <Sweeper.h>

Servo servo;
IMU imu;

uint32_t looptime = 0;

Sweeper sweep(1);


void setup() {
  // put your setup code here, to run once:
  imu.init();
  sweep.attach(10);
  Serial.begin(115200);
  sweep.enable = 1;
}

void loop() {
//   put your main code here, to run repeatedly:

  imu.readAccTempGyro();  
  int target = 15;
  target -= imu.correctError();
  target = map(target,0,38,1980,2456); 

//  Serial.println(target);
  
  sweep.update(target);
    
}
