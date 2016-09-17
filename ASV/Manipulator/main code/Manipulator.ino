#include <MANIPULATOR.h>

SWEEPER sweep1(5);
IMU imu(0x68);

void setup(){
  Serial.begin(115200);
  imu.Init();
  imu.correctToZero();          // run only when fitting to new position
  sweep1.Attach(9);             // pin connection
}

void loop(){
  int target_us=0,diff=0;
  
  imu.readAccTempGyro();
//  imu.printAccTempGyro();

  diff =(round)(975/90*imu.Error()+1525);
  
  sweep1.Update(target_us,diff);

  
//  imu.setting(); 
//  imu.CalibrateAcc();
//  imu.printAccTempGyro();
   
}



