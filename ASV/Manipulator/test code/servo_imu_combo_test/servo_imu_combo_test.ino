#include<Wire.h>
#include <math.h>
#include <Servo.h>
    
class Sweeper {
  Servo servo;
//  int pos;                        // current position  
//  int increment;                  // increment step size
  int updateInterval;             // interval between updates
  unsigned long lastUpdate;       // last update position
  int acw, cw;                    //  550 is 0 deg, 2500 is 180 deg  
                                  // wrt from x axis  1deg 10.83333333µs
public:                           // ********THIS IS POSITION************
  Sweeper(int interval) {
    updateInterval=interval;
  }

  void Attach(int pin) {
    servo.attach(pin);
  }

  void Detach() {
    servo.detach();
  }

  void Update(int diff) {
    if ((millis() - lastUpdate) > updateInterval) {
      lastUpdate = millis();      
      servo.writeMicroseconds(diff);  
    }                               
  }
};

const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
double x,y,z;
long int caliAx=0, caliAy=0, caliAz=0, caliGx=0, caliGy=0, caliGz=0;

int i=0;

double pitch;
double zero_pt;

//          servo reaction speed 6V no load 0.21s/60degree
//          servo 0-180 degree 550-2500

Sweeper sweep1(1);      //init servo object

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  correction();
  
  sweep1.Attach(7);                 // pin servo is attached

}
void loop(){

  readAccTempGyro(); 
//  printAccTempGyro();
  double temp = atan2(AcX,AcZ);
  temp=temp*180/3.1415926;
  
//  Serial.println("\n\n");
//  Serial.println(temp);
//  Serial.println("\n\n");
//
  int diff =(int)(975/90 *temp +1525);
//  
//  
  sweep1.Update(diff);
 
//  correction();

//  CalibrateAcc();
//  setting();
   
}

void correction(   ) {
  readAccTempGyro();
  pitch=atan2(x,z);
  pitch= pitch*180/3.14159265;  
  zero_pt=pitch;
  Serial.println(pitch);
}

void readAccTempGyro() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);     // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  x=AcX; y=AcY; z=AcZ;
//
//
                    AcX-=1826;  AcY+=440; AcZ-=390; GyX+=221; GyY-=51;  GyZ=GyZ;
//
//
}
 

void setting() {
  int gyro_config=readReg(0x1B);                    // check gyro and accel config setting
  int accel_config=readReg(0x1C);                   //  bit4 bit3,
  Serial.print("gyro config: ");                     
  Serial.print(gyro_config,BIN);                     //gyro=250 500 1000 2000       0x1B
  Serial.println("0=250, 01=500, 10=1000, 11=2000 degree/s");
  Serial.print("accel config: ");                    
  Serial.print(accel_config,BIN);                    //accel= 2 4 8 16              0x1C
  Serial.println("0=2, 01=4, 10=8, 11=16 g");
}

void printAccTempGyro() {
  
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);

}
void writeReg(int reg, int data) {
  Wire.beginTransmission(MPU_addr);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission(true);
}

byte readReg(int reg) {
  Wire.beginTransmission(MPU_addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,1,true);
  return Wire.read();
}

void CalibrateAcc() {
  
  caliAx+=AcX;            //here provides the calibration for imu's accelerometer
  caliAy+=AcY;            //by placing the accelerometer in different orientation 
  caliAz+=AcZ;            //to obtain 1g in the direction
  caliGx+=GyX;
  caliGy+=GyY;
  caliGy+=GyZ;
  
  if (i==99) {
    caliAx/=100;
    caliAy/=100;
    caliAz/=100;
    caliGx/=100;
    caliGy/=100;
    caliGz/=100;
    Serial.println();
    Serial.println(caliAx);
    Serial.println(caliAy);
    Serial.println(caliAz);
    Serial.println(caliGx);
    Serial.println(caliGy);
    Serial.println(caliGz);
    delay(30000);
  }

  i++;
    
// reference for use on imu
//http://kitsprout.logdown.com/posts/335386  
//http://www.starlino.com/imu_guide.html
//http://www.analog.com/media/en/technical-documentation/application-notes/AN-1057.pdf
}


