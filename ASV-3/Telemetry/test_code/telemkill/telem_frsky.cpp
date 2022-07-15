//===========================================
//
//    FRSKY FUNCTIONS FOR ASV 3 TElEMETRY
//
//===========================================

#include "telem_frsky.h"

// Get RSSI in DB, if above threshold then set hb for frsky
void frsky_get_rssi() {
  // Map RSSI from 1000 to 2000 duty cycle to 0 to 100 dB
  internalStats[RSSI_FRSKY] = calculate_rssi();
  if ((internalStats[RSSI_FRSKY] != 255) && (internalStats[RSSI_FRSKY] > RSSI_THRESHOLD)) {
    heartbeat_timeout[FRSKY] = millis();
  }
  else {
    internalStats[RSSI_FRSKY] = 0xFFFF;
  }
}

// get rssi and map from 0 to 100
int calculate_rssi() {
	// Map from [1000 to 2000] to [0 to 100]
	int cppm = constrain(frsky.get_ch(FRISKY_RSSI), 1500, 2000);
	cppm -= 1500;
	return map(cppm, 0, 500, 0, 100);
}


// Get control mode from frsky based on arm toggle
void frsky_get_controlmode() {
  if (frsky.get_ch(FRISKY_ARM) > 1800)
  {
    control_mode_frsky = MANUAL_FRSKY;
    #ifdef FRSKYDEBUG
    Serial.println("Manual Frsky");
    #endif 
  }
  else if (frsky.get_ch(FRISKY_ARM) > 1200)
  {
    control_mode_frsky = STATION_KEEP;
    #ifdef FRSKYDEBUG
    Serial.println("Station keep");
    #endif 
  }
  else
  {
    control_mode_frsky = AUTONOMOUS;
    #ifdef FRSKYDEBUG
    Serial.println("Autonomous");
    #endif 
  }
}

// Scale and batt capacity then send over i2c to DAC for transmission to x8r -> frsky
void frsky_send_batt_capacity() {
  uint32_t capacity;
  if (powerStats[BATT1_CAPACITY] == 0xFFFF) {
    capacity = powerStats[BATT2_CAPACITY];
  } else if (powerStats[BATT2_CAPACITY] == 0xFFFF) {
    capacity = powerStats[BATT1_CAPACITY];
  } else {
    capacity =  min(powerStats[BATT1_CAPACITY], powerStats[BATT2_CAPACITY]);
  }
  float scaled_capacity = capacity * 3.3 / 100;
  uint16_t DAC_input = scaled_capacity * 4096 / 5;
  Wire.beginTransmission(I2C_ADDR_DAC);
  Wire.write(DAC_input >> 8);     // top 4 bit of the 12bit voltage
  Wire.write(DAC_input & 0xFF);    // bot 8 bit of the 12bit voltage
  Wire.endTransmission(true);
}



// TODO remote kill
void frsky_get_kill() {
  if (frsky.get_ch(7) < 1000)
  {
    remotekill_frsky = false;
    #ifdef FRSKYDEBUG
    Serial.println("KILL");
    #endif 
  }
  else
  {
    remotekill_frsky = true;
    #ifdef FRSKYDEBUG
    Serial.println("ALIVE");
    #endif 
  }
}

// TODO get actuated hydrophone control
void frsky_get_hydrophone() {
}
