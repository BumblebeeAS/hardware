#include "SoftPWM.h"
///https://github.com/Palatis/arduino-softpwm
/* 
Parameter: CHANNEL - The channel number is used as an ID for the pin.

Parameter: PMODE - DDRx register of the pin's port. Append the port letter to DDR. 
For example: The Arduino UNO diagram shows that Arduino pin 13 is PB5 
which means that the port is B so you should use the value DDRB for that pin.

Parameter: PORT - The port of the pin. For example: Arduino pin 13 is PB5 
so you should use the value PORTB for that pin.

Parameter: BIT - The bit of the pin. For example: Arduino pin 13 is PB5 
so you should use the value PORTB5 for that pin.
 */

SOFTPWM_DEFINE_CHANNEL(22, DDRA, PORTA, PORTA0);  //Arduino pin 22
SOFTPWM_DEFINE_CHANNEL(23, DDRA, PORTA, PORTA1);  //Arduino pin 23
SOFTPWM_DEFINE_CHANNEL(24, DDRA, PORTA, PORTA2);  //Arduino pin 24

//Define the softPWM object with the default 256 PWM levels. (0-255)
SOFTPWM_DEFINE_OBJECT(255);

void setup() {
  Serial.begin(115200);

  // begin with 60hz pwm frequency
  Palatis::SoftPWM.begin(490);

  // print interrupt load for diagnostic purposes
  Palatis::SoftPWM.printInterruptLoad();
}

static volatile uint8_t v = 0;
void loop() {
  //SoftPWM.set(channel_idx, value) - Set the PWM level of the given channel.
  Palatis::SoftPWM.set(22, 150);
  Palatis::SoftPWM.set(23, 200);
  Palatis::SoftPWM.set(24, 50);
}
