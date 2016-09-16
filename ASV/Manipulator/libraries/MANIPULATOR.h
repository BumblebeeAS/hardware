#ifndef MANIPULATOR_H
#define MANIPULATOR_H


#include <Arduino.h>
#include <Servo.h>
#include <Wire.h>

class SWEEPER {
  Servo servo;
  int updateInterval;             // interval between updates
  unsigned long lastUpdate;       // last update position
  int target_us;                  //  550 is 0 deg, 2500 is 180 deg,1525 is 90deg ---!!!!!!!!---- different for each servo
  int error;                      // wrt from x axis  1deg 10.83333333µs
                                  //  y=975/90*x+1525
public:                           // ********THIS IS POSITION************
  SWEEPER(int interval);        
  void Attach(int pin);         
  void Detach();
  void Update(int target_us,int error);     
};          //  end servo class 


//          servo reaction speed 6V no load 0.21s/60degree      0.63s/180deg       3.5ms/deg
//          servo 0-180 degree 550-2500


class IMU {
  int MPU_addr;  // I2C address of the MPU-6050                        //            z      
  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;                                 //            ^    y                      +90deg
  unsigned long previousMillis;                                        //            |  /                        |
  double x,y,z;                                                        //            | /                         |
  long int caliAx, caliAy, caliAz, caliGx, caliGy, caliGz;             //            |------>  x           0deg  ------------------- > x
  double pitch;                                                        //                                        |
  double zero_pt;                                                      //                                        |
  double error;                                                        //                                        -90deg
    
public:
  IMU(int addr);

  void Init ();     // wake up IMU
  void setting();       // show current acc gyro range setting
  void Pitch();       //  read current pitch
  
  void readAccTempGyro();   // update value from IMU
  void printAccTempGyro();  // print on Serial Monitor

  void correctToZero();     // setting zero_pt
  double Error();           // correction
  void CalibrateAcc();      // loop for ave value of readings
  
  void writeReg(int reg, int data);    // write value into register
  uint8_t readReg(int reg);            // read value from register
   
};     // end Imu class
#endif
