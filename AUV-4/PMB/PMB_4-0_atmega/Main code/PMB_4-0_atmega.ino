
//to do list 
 //rearrange display later and a readjust units
 //later add on function
 //add documentation once working

#include "Arduino.h"
#include "defines.h"
#include <Wire.h>
#include <SSD1306_text.h>
#include <can.h>
#include "auv_4.0_can_def.h"
#include "BQ34110.h"
#include <Adafruit_ADS1015.h>

uint16_t capacity = 0;
uint16_t voltage = 0;
uint16_t consumption = 0;

float current = 0;
float temperature = 0;
float pressure = 0;

Adafruit_ADS1115 ads;
uint16_t adc1 = 0;
uint16_t charge_status = 0;
uint32_t oledLoop = 0;

uint32_t CanHeartbeatLoop = 0;
uint32_t CanStatusLoop = 0;


MCP_CAN CAN(CAN_CS_PIN);
SSD1306_text display(PIN_OLED_RESET);


void setup() {
  // put your setup code here, to run once:
    /* MCU Initialisation */
  pinMode(PIN_VEH_OFF, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_REED_OFF, INPUT);
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_VEH_OFF, HIGH);

  Serial.begin(115200);
  /* End of MCU Initialisation */

    /* CAN Initialisation*/

  CAN_init();
  // CAN Masking
  /* End of CAN Initialisation*/

  /* Peripherals Initialisation */
  Wire.begin();

  /* Display Init */
  Serial.println("Initialising Display");
  display.init();

  /* Pressure Sensor Init*/
  Serial.println("Initialising Pressure Sensor");
  ads.begin();

    /* Clear and update display with initial data */
  display.clear();
  updateDisplay();
//  Serial.println("Init fuel gauge");
//	BQ_Init();
//  Serial.println("BQ_init");
//	BQ_CalibrateVoltage(16520);
//  Serial.println("Voltage calibrated");
//	BQ_CalibrateCurrent(200);
//  Serial.println("Current calibrated");

	//BQ_Learning();
//	BQ_RestoreCCSettings();
// Serial.println("CC restored");
  
  digitalWrite(PIN_VEH_OFF, HIGH);
  Serial.println("pMOS on!");
 
}

void loop() {
  // put your main code here, to run repeatedly:
  //Check for soft off button
  if (digitalRead(PIN_REED_OFF)) {
    Serial.println("shutting down");
    digitalWrite(PIN_VEH_OFF, LOW);
    displayOffMessage();
    digitalWrite(PIN_RELAY, HIGH);
    /* PMB is turned off */
  }

  if ((millis() - CanHeartbeatLoop) > CAN_HEARTBEAT_INTERVAL) {
    publishCanHB();
    CanHeartbeatLoop = millis();
  }
  
  if ((millis() - CanStatusLoop) > CAN_STATUS_INTERVAL) {
    update_status();
    publishCanStatus();
    CanStatusLoop = millis();
  }

  if (millis() - oledLoop > OLED_INTERVAL) {
    updateDisplay();
    oledLoop = millis();
  }  


  
}



void updateDisplay() {              
  display.setTextSize(1, 1);
  display.setCursor(0, 0);
  display.write("Battery PMB ");
  display.print(PMB_NO);
  display.setCursor(1, 0);
  display.write("Batt %:      ");
  display.setCursor(1, 48);
  display.write("       ");
  display.setCursor(1, 48);
  display.print(capacity);
  display.setCursor(2, 0);
  display.write("Batt Volt: ");
  display.setCursor(2, 68);
  display.write("         ");
  display.setCursor(2, 68);
  display.print(voltage);
  display.setCursor(3, 0);
  display.write("Current: ");
  display.setCursor(3, 68);
  display.write("         ");
  display.setCursor(3, 68);
  display.print(current);
  display.setCursor(4, 0);
  display.write("State: ");
  display.setCursor(4, 38);
  display.write("              ");
  display.setCursor(4, 38);
  display.print(getBattState());
  display.setCursor(5, 0);
  display.write("Temp: ");
  display.print(temperature);
  display.setCursor(6, 0);
  display.write("Pres: ");
  display.print(pressure);
  display.setCursor(7, 0);
  display.write("Low Batt: ");
  //(batt_low) ? display.print("YES") : display.print("NO");
}

void displayOffMessage() {   
  display.clear();
  display.setTextSize(1, 1);
  display.setCursor(0, 0);
  display.write("Battery PMB ");
  display.print(PMB_NO);
  display.setCursor(4, 0);
  display.write("Turning off...");
}

void CAN_init()
{
START_INIT:
  if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
  {
    Serial.println("CAN BUS: OK");
  }
  else
  {
    Serial.println("CAN BUS: FAILED");
    Serial.println("CAN BUS: Reinitializing");
    delay(1000);
    goto START_INIT;
  }
  Serial.println("INITIATING TRANSMISSION...");
}

void publishCanStatus() {
  uint8_t BATT_stat[8] = { 0 };
  uint8_t PMB_stats[8] = { 0 };
  CAN.setupCANFrame(BATT_stat, 0, 2, (uint16_t)(current));
  CAN.setupCANFrame(BATT_stat, 2, 2, voltage);
  CAN.setupCANFrame(BATT_stat, 4, 2, capacity);
  CAN.setupCANFrame(BATT_stat, 6, 2, consumption);

  CAN.setupCANFrame(PMB_stats, 0, 2, (uint16_t)(temperature));
  CAN.setupCANFrame(PMB_stats, 2, 2, (uint16_t)(pressure));

  CAN.sendMsgBuf(CAN_BATT1_STAT, 0, 8, BATT_stat);
  CAN.sendMsgBuf(CAN_PMB1_STAT, 0, 4, PMB_stats);
}

void publishCanHB() {
  uint8_t HB[1] = { PMB_HEARTBEAT_ID }; //HEARTBEAT_PMB
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, HB);
}

double readInternalPressure() {
  /*
  VOUT = VS x (0.004 x P - 0.040)�� (Pressure Error x Temp Factor x 0.004 x VS)
  VS = 5.1 �� 0.36 Vdc
  */
  // internal   raw value 9690 = 1010mb = 101kPa  
  ads.set_continuous_conv(1);
  delay(ADS_DELAY);
  adc1 = ads.readADC_Continuous();
  //Serial.println((((double)adc1*0.0001875) / (Vref*0.0040) + 10));
  return (((double)adc1*0.0001875) / (Vref*0.0040) + 10); 
}

void update_status(){
  pressure = readInternalPressure();
//  current = BQ_GetCurrent();
  voltage = BQ_GetVoltage();  
//  consumption = BQ_GetRemainingCapacity();
//  capacity = BQ_GetRSOC();
//  temperature = (float)BQ_GetTemp();
}
  
String getBattState() {
  charge_status = BQ_GetBattStatus();
	if (charge_status & 0x02)
		return "DISCHARGE";
	else if (charge_status & 0x01)
		return "CHARGE";
	else
		return "DISCHARGE";
}
