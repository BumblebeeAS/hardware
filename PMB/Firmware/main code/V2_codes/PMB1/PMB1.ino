#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <TimerOne.h>
// #include <ros.h>
// #include <batt_msgs/Battery.h>
#include <Wire.h>


//cubic approximation coefficients
#define coef_a                2.801388537
#define coef_b                -208.6689126
#define coef_c                5196.993309
#define coef_d                -43182.43852

//timer interrupt interval
#define SAMPLING_INTERVAL     100000       // microseconds 

//current measurement caliberation
#define SHUNT_RESISTOR        0.002        // ohms 
#define CURRENT_RATIO         0.7216811031  
#define CURRENT_OFFSET        0.03

//voltage readings caliberation
#define cell6_adc_ratio       0.0332 
#define cell5_adc_ratio       0.197181814  
#define cell4_adc_ratio       0.244377275  
#define cell3_adc_ratio       0.325966493  
#define cell2_adc_ratio       0.497397968  
#define cell1_adc_ratio       0.993694855  

#define cell6_adc_offset       -0.2543  
#define cell5_adc_offset        0.05269355 
#define cell4_adc_offset       -0.1076     
#define cell3_adc_offset       -0.1076     
#define cell2_adc_offset       0.0078684   
#define cell1_adc_offset       0.00739624  

//maximum battery capacity
#define maxCapacity             6500.0

boolean toggle=false;
uint16_t temp0=0, temp1=0;
float voltage0, voltage1;
float current;
float temp=0;
float initialCapacity=6500.0;     //mAh
float initialPercentage=100.0;
float currentCapacity;
float currentPercentage=100.0;
float usedCapacity=0;
int memCell=4;
int writeCount=0;
unsigned int tick       =       0;
unsigned int second     =       0;
float cell_voltage[6]   =       {0.0, 0.0, 0.0,
                                 0.0, 0.0, 0.0};
float analog_sensors[6] =       {0, 0, 0, 0, 0, 0};
float Vcc               =       5.0; //base, to avoid divide by zero error
byte highbyteAIN0, lowbyteAIN0, highbyteAIN1, lowbyteAIN1;

float current_array[5]={0.0,0.0,0.0,0.0,0.0};
int current_index=0;
float temp_holder;
float current_filtered=0.0;

//serial stuff
//make true to output ascii, false to output bitstream
bool ascii = true;

int int_pressure = 0;
//cell Volts  6x2 (3700-4200)
int int_CV1 = 0;
int int_CV2 = 0;
int int_CV3 = 0;
int int_CV4 = 0;
int int_CV5 = 0;
int int_CV6 = 0;
//current     x2 (0-15000)
int int_curr = 0;
//capacity    x1 (0-100)
int int_rem_cap_percent = 0;
//used mAh    x2 (0-65536)
int int_used_cap_mah = 0;
//temp x1     (0-255)
int int_temp = 0;

// ros::NodeHandle nh;
// batt_msgs::Battery battery_msg;
// ros::Publisher pmb("battery_status", &battery_msg);

void flash_LED()
{
  for(int i=3; i<=10; i++)
    pinMode(i,OUTPUT);
  
  for(int i=0;i<3;i++)
  {
    display_LEDs(10);
    delay(500);
    display_LEDs(1);
    delay(500);  
  }
}

void blink_LEDs()
{
  if(tick%2==0)
    if(toggle)
    {
      display_LEDs(10);
      toggle=!toggle;
    }
      else
      {
        display_LEDs(2);
        toggle=!toggle;
      }
}

void update_LEDs(float voltage)
{       
         //no LEDs 
        if(voltage<22.75)
          display_LEDs(2);
         //1 LED 
        else if(voltage<23.0)
          display_LEDs(3);
         //2 LEDs
        else if(voltage<23.25)
          display_LEDs(4);
         //3 LEDs
        else if(voltage<23.5)
          display_LEDs(5);
         //4 LEDs
        else if(voltage<23.75)
          display_LEDs(6);
         //5 LEDs
        else if(voltage<24.0)
          display_LEDs(7);  
         //6 LEDs 
        else if(voltage<24.25)
          display_LEDs(8);
         //7 LEDs
        else if(voltage<24.5)
          display_LEDs(9);
        //8 LEDs  
        else
          display_LEDs(10);          
          
}

void display_LEDs(int pin)
{
	for(int i=3; i<=pin; i++)
		digitalWrite(i, HIGH);
	for(int i=pin+1; i<=10; i++)
		digitalWrite(i, LOW);
}

void begin_ads1115()
{
  Wire.begin();                 // join i2c bus (address optional for master)
  Wire.beginTransmission(72);   // transmit to device #72 (0b1001000)  
  Wire.write(1);                // point register set to config register
  Wire.write(194);              // configure ADS chip, this if the MSByte of the 16bit config register
  Wire.write(227);              // LSByte of the 16bit config register 
  Wire.endTransmission();       // stop transmitting   
  Wire.beginTransmission(72);   // transmit to device #72 (0b1001000)  
  Wire.write(0);                // point register set to config register
  Wire.endTransmission();       // stop transmitting 
  
}

void read_voltages()
{ 
    if(tick % 6 == 1)
        analog_sensors[5] = analogRead(7);
//    if(tick % 6 == 2)
//        analog_sensors[4] = analogRead(6);
//    if(tick % 6 == 3)
//        analog_sensors[3] = analogRead(3);
//    if(tick % 6 == 4)
//        analog_sensors[2] = analogRead(2);
//    if(tick % 6 == 5)
//        analog_sensors[1] = analogRead(1);
//    if(tick % 6 == 0)
//        analog_sensors[0] = analogRead(0);

    cell_voltage[5] = ((analog_sensors[5] * cell6_adc_ratio) + cell6_adc_offset);
    cell_voltage[4] = ((analog_sensors[4] * cell5_adc_ratio) + cell5_adc_offset);
    cell_voltage[3] = ((analog_sensors[3] * cell4_adc_ratio) + cell4_adc_offset);
    cell_voltage[2] = ((analog_sensors[2] * cell3_adc_ratio) + cell3_adc_offset);
    cell_voltage[1] = ((analog_sensors[1] * cell2_adc_ratio) + cell2_adc_offset);
    cell_voltage[0] = ((analog_sensors[0] * cell1_adc_ratio) + cell1_adc_offset);            
}

void read_AIN0()
{
  Wire.requestFrom(72, 2);    // request 6 bytes from slave device #72 (0b1001000)
  while(Wire.available())     // slave may send less than requested
  { 
    highbyteAIN0 = Wire.read(); 
    lowbyteAIN0 = Wire.read();
  } 
  
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(1);               // point register set to config register
  Wire.write(82);              // configure ADS chip, this is the MSByte of the 16bit config register
  Wire.write(227);             // LSByte of the 16bit config register  
  Wire.endTransmission();      // stop transmitting  
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(0);               // point register set to config register  
  Wire.endTransmission();      // stop transmitting  
}

void read_AIN1()
{
  Wire.requestFrom(72, 2);    // request 6 bytes from slave device #72 (0b1001000)
  while(Wire.available())     // slave may send less than requested
  { 
    highbyteAIN1 = Wire.read(); 
    lowbyteAIN1 = Wire.read();
  } 

  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(1);               // point register set to config register
  Wire.write(66);              // configure ADS chip, this is the MSByte of the 16bit config register
  Wire.write(227);             // LSByte of the 16bit config register  
  Wire.endTransmission();      // stop transmitting
  Wire.beginTransmission(72);  // transmit to device #72 (0b1001000)  
  Wire.write(0);               // point register set to config register  
  Wire.endTransmission();      // stop transmitting

}

void calculate_current()
{
    read_AIN0();
    delay(5);
    read_AIN1();
    
    /* calculate voltage at nodes of the shunt resistor and hence current through */
   if((highbyteAIN0>>7)&0x1==1)
     temp0=0;
     else
     {
         temp0 = highbyteAIN0 * 256;
         temp0 = temp0 + lowbyteAIN0;  
     }
   voltage0=temp0;
   voltage0=voltage0/32767*4.096/40;

    if((highbyteAIN1>>7)&0x1==1)
     temp1=0;
     else
     {
         temp1 = highbyteAIN1 * 256;
         temp1 = temp1 + lowbyteAIN1;  
     }
   voltage1=temp1;
   voltage1=voltage1/32767*4.096/40;   
      
   current=((voltage1-voltage0)/SHUNT_RESISTOR*CURRENT_RATIO)+CURRENT_OFFSET;
   
   current_array[current_index]=current;
   current_index++;
   if(current_index==5)
   {
     median_filter();
     current_index=0;
   }
}

void median_filter()
{
  for(int i=0; i<5; i++)
    for(int j=i+1; j<5; j++)
      if(current_array[i]>current_array[j])
      {
        temp_holder=current_array[i];
        current_array[i]=current_array[j];
        current_array[j]=temp_holder;
      }
      
  current_filtered=current_array[2];
}
  
void calculate()
{
    //integrate mAh used, update current capacity and percentage  
    usedCapacity+=current*SAMPLING_INTERVAL/1000.0/3600.0;                 //in mAh
    currentCapacity=initialCapacity-usedCapacity;                          //in mAh
    currentPercentage=currentCapacity/65.0;                                 //in %
    
    //perform this check at every 5s
    if(tick%50==0)
    {           
      
      //store current percentage into EEPROM   
      if(EEPROM.updateLong(memCell, int(currentPercentage*100)))
      {
        writeCount++;
        if(writeCount>=10)
        {
          writeCount=0;
          memCell+=4;
          EEPROM.writeLong(memCell, int(currentPercentage*100));
          if(memCell>=(511-4))  //outside of memory range 0-511 bytes, each int long takes 4 bytes
            memCell=4;
           EEPROM.writeInt(2,memCell);  
        }
      }      
    }
    
    //alert when percentage is low or voltage is low
    if(cell_voltage[5] < 22.5)
      blink_LEDs();
      else
        update_LEDs(cell_voltage[5]);
    
    if(tick%10==0)
      second++;
    
    //increment tick counter 
    tick++;
}

void gauge_capacity()
{
  for(int i=0; i<10; i++)
  {  
    analog_sensors[5] = analogRead(7);
    cell_voltage[5] = ((analog_sensors[5] * cell6_adc_ratio) + cell6_adc_offset);
//    Serial.println(cell_voltage[5]);
    delay(50);
  }

  for(int i=0; i<20; i++)
  {  
    analog_sensors[5] = analogRead(7);
    cell_voltage[5] = ((analog_sensors[5] * cell6_adc_ratio) + cell6_adc_offset);
//    Serial.println(cell_voltage[5]);
    temp+=cell_voltage[5];
    delay(50);
  }
  temp=float(temp/20.0);  
  initialPercentage = (pow(temp,3)*coef_a + pow(temp,2)*coef_b + temp*coef_c + coef_d);
  initialCapacity = initialPercentage/100.0*maxCapacity;

}

void read_stored_percentage()
{
//  Serial.print("reading eeprom at:");
//  Serial.println(memCell);
  initialPercentage=float(EEPROM.readLong(memCell))/100.0;    //read capacity from that index 
  initialCapacity = initialPercentage/100.0*maxCapacity;
}

void writeIntSerial(int data, int len){
  if(ascii){
    Serial.print(data);
  } else {
    char* a = (char*)&data;  
    int sent= Serial.write(a);

    //to make up for remaining bytes
    if(sent<len){
      for(int i=0; i<(len-sent); i++){
        Serial.write(0);
      }
    }
  }
  
}

void publishSerial(){
  //everything written must be an int, so convert first in loop
  
  //start byte
  Serial.write('#');
  //pressure    x1 (0-255)
  writeIntSerial(int_pressure, 1);
  //cell Volts  6x2 (3700-4200)
  writeIntSerial(int_CV1, 2);
  writeIntSerial(int_CV2, 2);
  writeIntSerial(int_CV3, 2);
  writeIntSerial(int_CV4, 2);
  writeIntSerial(int_CV5, 2);
  writeIntSerial(int_CV6, 2);
  //current     x2 (0-15000)
  writeIntSerial(int_curr, 2);
  //capacity    x1 (0-100)
  writeIntSerial(int_rem_cap_percent, 1);
  //used mAh    x2 (0-65536)
  writeIntSerial(int_used_cap_mah, 2);
  //temp x1     (0-255)
  writeIntSerial(int_temp, 1);

}

void setup(){
  
    // nh.initNode();
    // nh.advertise(pmb);
        
    analogReference(INTERNAL);
    Serial.begin(9600);   
//    Serial.println("hello world");    
    begin_ads1115();

    calculate_current();
    // battery_msg.cell2=current;      //debug
//    Serial.println(current);
                     
    //extract memory location where batt percentage is last stored 
    memCell=EEPROM.readInt(2);
    if(memCell<4 || memCell>511)
    {
      memCell=4;
      EEPROM.writeInt(2,memCell);
    }
 
    //checking if there is a load current
    if(current < 0.1)
    {  
//      Serial.println("gauging");
      gauge_capacity();
      // battery_msg.cell4=0;  //debug
    }
      else
      {
//        Serial.println("reading eeprom");
        read_stored_percentage();
        // battery_msg.cell4=1;  //debug
      }
      
    flash_LED();
    Timer1.initialize(SAMPLING_INTERVAL);
    Timer1.attachInterrupt(calculate);     
}
 
void loop(){
  
    read_voltages();
    calculate_current();

    //converting to int
    int_pressure = 0;
    //cell Volts  6x2 (3700-4200)
    int_CV1 = (int)(cell_voltage[0] * 1000);
    int_CV2 = (int)(cell_voltage[1] * 1000);
    int_CV3 = (int)(cell_voltage[2] * 1000);
    int_CV4 = (int)(cell_voltage[3] * 1000);
    int_CV5 = (int)(cell_voltage[4] * 1000);
    int_CV6 = (int)(cell_voltage[5] * 1000);
    //current     x2 (0-15000)
    int_curr = (int)(current_filtered * 1000);
    //capacity    x1 (0-100)
    int_rem_cap_percent = (int)currentPercentage;
    //used mAh    x2 (0-65536)
    int_used_cap_mah = (int)usedCapacity;
    //temp x1     (0-255)
    int_temp = (int)temp;

    publishSerial();
//     battery_msg.cell1 = initialPercentage;    //debug
//  //   battery_msg.cell2 = cell_voltage[1];
//     battery_msg.cell3 = second;
// //    battery_msg.cell4 = cell_voltage[3];
//     battery_msg.cell5 = memCell;             //debug 
//     battery_msg.cell6 = cell_voltage[5];             
//     battery_msg.vcc   = temp;                //debug
//     battery_msg.current = current_filtered;
//     battery_msg.used_mAh = usedCapacity;
//     battery_msg.battery_percentage = currentPercentage;
//        Serial.print(temp);
//        Serial.print(" ");
//        Serial.print(writeCount);
//        Serial.print(" ");        
//        Serial.print(initialPercentage);
//        Serial.print(" ");
//        Serial.print(memCell);
//        Serial.print(" ");
//        Serial.print(currentPercentage);
//        Serial.print(" ");
//        Serial.println(current_filtered);
//        Serial.print(" ");
//        Serial.print(analog_sensors[5]);
//        Serial.print(" ");
//        Serial.println(cell_voltage[5]);


      // pmb.publish(&battery_msg);
      // nh.spinOnce();
      
      delay(100);
}
