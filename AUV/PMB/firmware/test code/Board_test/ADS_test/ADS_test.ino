
#include <ADS1115_min.h>
//#include <Adafruit_ADS1015.h>
#define vehiclePower 6
#define boardPower 3

ADS1115 testADC(72);
//Adafruit_ADS1115 testADC(72);

void setup(){
	Serial.begin(9600);
  testADC.init();
//  testADC.begin();
  pinMode(vehiclePower, OUTPUT);
  digitalWrite(vehiclePower, HIGH);
  pinMode(boardPower, OUTPUT);
  digitalWrite(boardPower, LOW);
}

void loop(){
   Serial.print("raw ADC0: ");
   Serial.print(testADC.readChannel(0));
//  Serial.print(testADC.readADC_SingleEnded(0));
   delay(10);
   Serial.print(" raw ADC1: ");
   Serial.print(testADC.readChannel(1));
//  Serial.print(testADC.readADC_SingleEnded(1));
   delay(10);
   Serial.print(" raw ADC2: ");
   Serial.print(testADC.readChannel(2));
//  Serial.print(testADC.readADC_SingleEnded(2));
   delay(10);   
   Serial.print(" raw ADC3: ");
   Serial.println(testADC.readChannel(3));
//  Serial.println(testADC.readADC_SingleEnded(3));
   delay(1000);
}
