#include <mcp_can.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include <Wire.h>

Adafruit_ADS1115 ADS75(75);
uint16_t ADC_result;
uint8_t buf[1];
unsigned char len = 0;
float current;
float batt1;
float batt2;
float rail24v;
float rail12v;
float rail5v;

MCP_CAN CAN(8);

void setup()
{
      pinMode(A0, OUTPUT);  //power control pin TH
      //DDRD |= (1<<DDF0);  //power control pin TH
      DDRD |= (1<<DDD6);  //power control pin TE
      DDRD |= (1<<DDD7);  //power control pin SA
      
      PORTF |= 0x01;   //enable Thruster power
      PORTD |= (1<<DDD7);   //enable SA power
      PORTD |= (1<<DDD6);   //enable TE power
      
      
  analogReference(INTERNAL2V56);
  ADS75.begin();
  Serial.begin(9600);
  START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}

uint8_t stmp[3];
void loop()
{
    //SENDING CURRENT READING  
    ADC_result = ADS75.readADC_SingleEnded(2);
    if (ADC_result > 32000)
      ADC_result = 0;
    current = ADC_result * 0.0019 + 0.0694;  //calibration 
  
    stmp[0] = current;
    stmp[1] = int((current*10)) % 10;
    stmp[2] = int((current*100)) % 10;  
    
    // send data:  id = 0x00, standrad flame, data len = 8, stmp: data buf
    CAN.sendMsgBuf(0x00, 0, 3, stmp);
//    Serial.print(" current ");
//    Serial.print(current,3);
    
    //SENDING VOLTAGE READINGS
    batt1 = analogRead(A2) * 0.0979 - 0.0927;
    stmp[0] = batt1;
    stmp[1] = int((batt1*10)) % 10;
    stmp[2] = int((batt1*100)) % 10;  
    CAN.sendMsgBuf(0x01, 0, 3, stmp);
//    Serial.print(" batt1 ");
//    Serial.print(batt1);

    batt2 = analogRead(A1) * 0.0979 - 0.0927;
    stmp[0] = batt2;
    stmp[1] = int((batt2*10)) % 10;
    stmp[2] = int((batt2*100)) % 10;  
    CAN.sendMsgBuf(0x02, 0, 3, stmp);
    Serial.print(" batt2 ");
    Serial.print(analogRead(A1));    
    
    rail24v = analogRead(A7) * 0.0655 + 0.5509;
    stmp[0] = rail24v;
    stmp[1] = int((rail24v*10)) % 10;
    stmp[2] = int((rail24v*100)) % 10;  
    CAN.sendMsgBuf(0x03, 0, 3, stmp);
//    Serial.print(" 24v ");
//    Serial.print(rail24v);
    
    rail5v = analogRead(A4) * 0.0144 + 0.0749;
    stmp[0] = rail5v;
    stmp[1] = int((rail5v*10)) % 10;
    stmp[2] = int((rail5v*100)) % 10;  
    CAN.sendMsgBuf(0x04, 0, 3, stmp);    
//    Serial.print(" 5V ");
//    Serial.print(rail5v);


    rail12v = analogRead(A6) * 0.0481 + 1.4828;
    if (rail12v <= 0)
      rail12v = 0.0;
    stmp[0] = int(rail12v);
    stmp[1] = int((rail12v*10)) % 10;
    stmp[2] = int((rail12v*100)) % 10;  
    CAN.sendMsgBuf(0x05, 0, 3, stmp);
    Serial.print(" 12V ");
    Serial.println(analogRead(A6));
    
    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        //canID = CAN.getCanId();
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        if (CAN.getCanId() == 6)
        {
            if (buf[0] == 1)
            {
              Serial.println("toggling Thruster board");
              if ((PORTF & 0x01) == 1)
                PORTF &= 0xFE;   //disable Thruster power 
              else 
                PORTF |= 0x01;   //enable Thruster power
            }
            
            if (buf[0] == 3)
            {
              Serial.println("toggling TE board");
              if ((PORTD >> DDD6) & 0x01 == 1)
                PORTD &= 0b10111111;   //disable TE power 
              else 
                PORTD |= (1<<DDD6);   //enable TE power
            }
            
            if (buf[0] == 2)
            {
              Serial.println("toggling SA board");
              if ((PORTD >> DDD7) & 0x01 == 1)
                PORTD &= 0b01111111;   //disable SA power
              else 
                PORTD |= (1<<DDD7);   //enable SA power
            }
        }
    }
    
    delay(100);                       // send data per 100ms
}
