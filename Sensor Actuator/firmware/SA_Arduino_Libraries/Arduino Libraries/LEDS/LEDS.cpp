#include <LEDS.h>


LEDS::LEDS(int pin1,int pin2,int pin3)
{
   RedPin=pin1;
   GreenPin=pin2;
   BluePin=pin3;

  pinMode(RedPin, OUTPUT);
  pinMode(GreenPin, OUTPUT);
  pinMode(BluePin, OUTPUT);
}

void LEDS::setcolour(int red, int green, int blue)
{
      analogWrite(RedPin, red);
      analogWrite(GreenPin, green);
      analogWrite(BluePin, blue);

}

void LEDS::colour(int col)
{
    switch (col) 
    {
    case 1://Red
      setcolour(255,0,0);
      break;
    case 2://Orange
      setcolour(255,35,0);     
      break;
    case 3://Yellow
      setcolour(255,60,0);
      break;
    case 4://Green
      setcolour(0,255,0);
      break;
    case 5://Blue
      setcolour(0,0,255);
      break;
    case 6://indigo
      setcolour(25,10,200);
      break;
    case 7://violet
      setcolour(50,0,80);
      break;    
    case 8://white
      setcolour(255,255,255);
      break; 
    case 9://off
      setcolour(0,0,0);
      break;
    case 10://pink
      setcolour(255,0,100);
      break;

  }
}


