//###################################################
//###################################################
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####
//
////Sensor & Actuator Board for BBAUV 3.0
//Firmware Version :             v2.0
//
// Written by Louis Goh
// Change Log for v1.2:
//###################################################
//###################################################
//###################################################

#include "sensors_actuators.h"
#include "define.h"
#include "can_defines.h"

//Constant declaration
#include <Arduino.h>
#include <Adafruit_ADS1015.h> //Display
#include <Wire.h> //For I2C
#include <Servo.h>
#include <HIH613x.h>
#include <LEDS.h>

#include <SPI.h> // for CAN Controller
#include <can.h>
#include <can_defines.h>

#include <stdint.h>
//Constants definition
#define PRESSURE_TYPE PRESSURE_TYPE_GAUGE_30
#define DEBUG_MODE NORMAL
#define VERSION 1.2

//Timming variables - to ensure the loop run at correct frequency
static uint32_t loopTime;
static uint32_t currentTime;

static uint32_t PressureFilterLoop_10;//10ms loop External pressure samples
static uint32_t ExterPress_loop_50;//50ms loop Publish External pressure
static uint32_t Mani_loop_100;//100ms loop Manipulators
static uint32_t Statues_loop_250;//Break 1000ms loop into 4 parts
static uint32_t LEDArray_loop_500;//500ms loop LED array 


//ADC definitions
//Swap between ADS_ADD and ADS_ADD_BACKUP
Adafruit_ADS1115 ads1115_ext(ADS_ADD_EXT); // ADS_ADD for external pressure         
Adafruit_ADS1115 ads1115_int(ADS_ADD_INT); // ADS_ADD for internal pressure
HIH613x humid(0x27);                 //Humidity Address 0x27 TempHumAddr
LEDS led(LED_Red_1,LED_Blue_1,LED_Green_1); //LED constructor  9 , 7 , 11
LEDS led2(LED_Red_2,LED_Blue_2,LED_Green_2);//LED constructor  10, 4 , 12
MCP_CAN CAN(CAN_Chip_Select);               //Set Chip Select to pin 8

boolean Pub_Press = false;
boolean FLAGMsg = false;

uint8_t LED1_buf, LED2_buf;//buffer for LEDs
uint8_t state = 0;        //State for 250ms loop
uint8_t CAN_State_Buf[3]; //CAN tx & error statues buffer

unsigned char len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message
uint8_t Manipulator_buf1, Manipulator_buf2, Mani_Temp;//Buffers for manipulators trigger
uint8_t StatuesBuf[3];//Temp humidity and internal pressure buffer
uint8_t PressureBuf[2];//external pressure buffer
int16_t pressure;//Pressure Sensor Definitions


void setup()
{
	//Debug Mode: to be removed by the compiler if not in debug mode.
	#if DEBUG_MODE == DEBUG_INFO
	  Serial.begin(115200);
	  Serial.println("Debug Mode INFO");
	#endif
	  Serial.begin(115200);
	  Serial.println("NORMAL Mode INFO");
	CAN_init();
	leds_init();
	Serial.println("LEDS: OK");
    manipulators_init();
	Serial.println("Manipulators: OK");
    pressure_init();
	Serial.println("Pressure: OK");

	Serial.println("ALL OK");
  //Initialize MainLoop Timing variables
	currentTime = loopTime = PressureFilterLoop_10 = ExterPress_loop_50 = Mani_loop_100 = Statues_loop_250 = LEDArray_loop_500 = millis();
}

int counter = 0;
void loop()
{
   currentTime=millis();  
   if(currentTime > ExterPress_loop_50 + 50){
       
       Pub_Press = true;
       //publish pressure data
	   CAN.setupCANFrame(PressureBuf,0,2,pressure);
	   CAN.sendMsgBuf(CAN_pressure, 0, 2, PressureBuf);
       //Serial.print(PressureBuf[0]);
       //Serial.print("  ");
       //Serial.println(PressureBuf[1]);
       
       #if DEBUG_MODE == DEBUG_INFO
		  Serial.print("p:");
          Serial.println(millis() - currentTime);//change the debug mode
        #endif
        ExterPress_loop_50 = currentTime;

     }
    
    currentTime=millis();  
    if(currentTime > Statues_loop_250 + 250){
            switch(state){
               case 0:
                 //start I2C Read
                 humid.measurementRequest();
                 Pub_Press = true;
				 checkCANmsg();
				 break;
               case 1:
                 //Get I2C Data
                 //push into send state buf
                 humid.dataFetch();
                 StatuesBuf[0] = humid.getTemperature() + 0.5;
                 StatuesBuf[1] = humid.getHumidity() + 0.5;
                 break;                 
               case 2:
                 //InternalPressure Reading
                 StatuesBuf[2] = (byte)readInternalPressure();
                 Pub_Press = true;
                break;                 
               case 3:
                 //Publish message onto the CAN bus
				 CAN.sendMsgBuf(CAN_SA_stats, 0, 3, StatuesBuf);
                 //Serial.println(StatuesBuf[0]);
                 //Serial.println(StatuesBuf[1]);
                 //Serial.println(StatuesBuf[2]);
                 CAN_State_Buf[0]=CAN.checkError();
                 CAN_State_Buf[1]=CAN.checkTXStatus(0);//check buffer 0
                 CAN_State_Buf[2]=CAN.checkTXStatus(1);//check buffer 1
                 //Serial.println(CAN_State_Buf[0]);
                 //Serial.println(CAN_State_Buf[1]);
                 //Serial.println(CAN_State_Buf[2]);
				 CAN.sendMsgBuf(CAN_SA_BUS_stats, 0, 3, CAN_State_Buf);
                 break;   
            }
              if(state != 4)  state++;
                 else  state = 0;
              #if DEBUG_MODE == DEBUG_INFO
				 Serial.print("ones:");
				 Serial.println(millis() - currentTime);//change the debug mode
              #endif
             Statues_loop_250 = currentTime;
     }
      
  currentTime=millis();  
  if( currentTime >= (PressureFilterLoop_10 + 10))
  {       if(Pub_Press == false){
            readPressureFilter();
            #if DEBUG_MODE == DEBUG_INFO
			Serial.print("pf:");  
			  Serial.println(millis() - currentTime);//change the debug mode
            #endif
    
            PressureFilterLoop_10 = currentTime;
        }else{
            Pub_Press = false; 
            PressureFilterLoop_10 = currentTime;
        }
  }
  
    currentTime=millis();       
    if(currentTime > Mani_loop_100 + 100){
       PORTC = Manipulator_buf1;//pin 2 is not being used 
       //Mani_Temp = PORTG & B11111100;
       Manipulator_buf2 = Manipulator_buf2 & B00000011;//Pin 0 and 1
      // PORTG = Mani_Temp | Manipulator_buf2;
       #if DEBUG_MODE == DEBUG_INFO
	   Serial.print("ma:");   
	   Serial.println(millis() - currentTime);//change the debug mode
       #endif
       Mani_loop_100 = currentTime;
     }


     currentTime=millis();  
     if(currentTime > LEDArray_loop_500 + 500){
            led.colour(LED1_buf);
	    led2.colour(LED2_buf);
              #if DEBUG_MODE == DEBUG_INFO
				Serial.print("le:");
				Serial.println(millis() - currentTime);//change the debug mode
              #endif
            LEDArray_loop_500 = currentTime;
     }
}

void CAN_init(){
START_INIT:
    if(CAN_OK == CAN.begin(CAN_1000KBPS)){                   // init can bus : baudrate = 500k
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init: OK");
        #endif           
    }
    else{
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init: FAILED");
          Serial.println("reinitialising CAN...");
          delay(1000);
        #endif           
        goto START_INIT;
    }

    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
    
    //register number, extension, 
	CAN.init_Filt(0, 0, CAN_pressure);//Pressure
	CAN.init_Filt(1, 0, CAN_manipulator);//Manipulator
	CAN.init_Filt(2, 0, CAN_LED);//LED Array
	CAN.init_Filt(3, 0, CAN_SA_stats);//Sensor & Actuator Statistics 
	CAN.init_Filt(4, 0, CAN_SA_BUS_stats);//Sensor & Actuator CAN Stats

    //Interrupt not working yet 0 for Pin D2, Interrupt service routine ,Falling edge 

}

void leds_init()
{
	for(int i=0;i<10;i++)
	{
		led.colour(i);
		led2.colour(i);
		delay(200);
	}
	delay(1000);
}

void humid_init()
{
	humid.measurementRequest();
        //push data out
        state = 0;
	delay(100);
}
void pressure_init()
{
	ads1115_int.begin();
	Serial.print("int:");
	//Serial.println(ads1115_int.set_continuous_conv(1);
	ads1115_ext.begin();
	Serial.print("ext:");
	Serial.println(ads1115_ext.set_continuous_conv(0));
	delay(100);
	if(readPressure() < 3000)
	{
		Serial.println("PRES NOT OK");
	}
	delay(100);
}

void manipulators_init()
{
	//Initialize Manipulators
	pinMode(MANI_1,OUTPUT);
	pinMode(MANI_2,OUTPUT);
	pinMode(MANI_3,OUTPUT);
	pinMode(MANI_4,OUTPUT);
	pinMode(MANI_5,OUTPUT);
	pinMode(MANI_6,OUTPUT);
	pinMode(MANI_7,OUTPUT);
	pinMode(MANI_8,OUTPUT);
	pinMode(MANI_9,OUTPUT);
}

//Sensor Tasks Callbacks ---------------------------------------------------------
uint16_t readPressure()
{
    uint16_t adc;
	ads1115_int.set_continuous_conv(0);
	adc = ads1115_int.readADC_Continuous();
	Serial.print("ep:");
	Serial.println(adc);
	return adc;
}

int16_t readInternalPressure()
{
    int16_t InternalPress;
	ads1115_int.set_continuous_conv(1);
	delay(5);
	InternalPress = ads1115_int.readADC_Continuous();
	ads1115_int.set_continuous_conv(0);
	delay(5);
	Serial.print("ip:");
	Serial.println(InternalPress);
	InternalPress = ((float)InternalPress*0.0001875) / (INTPRES_REF*0.0040) + 10;
 return InternalPress;
}

/* Discrete Low Pass Filter to reduce noise in signal */
void readPressureFilter()
{
   int16_t temp = readPressure();
   if(temp != 0)
   {
	   pressure = pressure + LPF_CONSTANT*(float)(temp - pressure);
   }
}

void checkCANmsg()
{
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive())
	{
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		switch (CAN.getCanId()){
		case CAN_manipulator:{//ID = 3 (number)
			//put into manipulator buffer
			Manipulator_buf1 = buf[0];
			Manipulator_buf2 = buf[1];
			Serial.println("mani!");
			Serial.print(buf[0]);
			Serial.print(" ");
			Serial.println(buf[1]);
			//Return acknowledgement
			buf[2] = 1;
			buf[3] = buf[0]^buf[1] ; //XOR Checksum
			CAN.sendMsgBuf(CAN.getCanId(), 0, len + 1, buf);
			break;
		}

		case CAN_LED:{//ID = 9 (number)
			//put in the LED buffer
			LED1_buf = buf[0];
			LED2_buf = buf[1];
			break;
		}
		default:{
			//TODO=>throw an error 
		}

		}
		CAN.clearMsg();
	}
}