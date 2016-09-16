#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include <Servo.h>
#include <Wire.h>

class SWEEPER {
  Servo servo;
  int updateInterval;             // interval between updates
  unsigned long lastUpdate;       // last update position
  int target_us;                  //  550 is 0 deg, 2500 is 180 deg,1525 is 90deg ---!!!!!!!!---- different for each servo
  int error;                      // wrt from x axis  1deg 10.83333333µs

public:                           // ********THIS IS POSITION************
  SWEEPER(int interval);
  void Attach(int pin);
  void Detach();
  void Update(int target_us,int error);
};          //  end servo class 

class IMU {
  int MPU_addr;  // I2C address of the MPU-6050                               //            z      
  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;                                              //            ^    y                      +90deg
  unsigned long previousMillis;                                                     //            |  /                        |
  double x,y,z;                                                                     //            | /                         |
  long int caliAx, caliAy, caliAz, caliGx, caliGy, caliGz;                          //            |------>  x           0deg  ------------------- > x
  double pitch;                                                                     //                                        |
  double zero_pt;                                                                   //                                        |
  double error;                                                                     //                                        -90deg
    
public:
  IMU(int addr);

  void IMU_Init ();
  void setting();
  
  void readAccTempGyro();
  void printAccTempGyro();

  void correctToZero();
  double Error();
  void CalibrateAcc();
  
  void writeReg(int reg, int data);
  uint8_t readReg(int reg);
   
};     // end Imu class
#endif